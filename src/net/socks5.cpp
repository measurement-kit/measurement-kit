// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/net/socks5.hpp"
#include "src/net/connect.hpp"

namespace mk {
namespace net {

Socks5::Socks5(Var<Transport> tx, Settings s, Var<Reactor>, Var<Logger> lp)
    : Emitter(lp), settings(s), conn(tx),
      proxy_address(settings["net/socks5_address"]),
      proxy_port(settings["net/socks5_port"]) {
    socks5_connect_();
}

Buffer socks5_format_auth_request(Var<Logger> logger) {
    Buffer out;
    out.write_uint8(5); // Version
    out.write_uint8(1); // Number of methods
    out.write_uint8(0); // "NO_AUTH" meth.
    logger->debug("socks5: >> version=5");
    logger->debug("socks5: >> number of methods=1");
    logger->debug("socks5: >> NO_AUTH (0)");
    return out;
}

ErrorOr<bool> socks5_parse_auth_response(Buffer &buffer, Var<Logger> logger) {
    auto readbuf = buffer.readn(2);
    if (readbuf == "") {
        return false; // Try again after next recv()
    }
    logger->debug("socks5: << version=%d", readbuf[0]);
    logger->debug("socks5: << preferred_auth=%d", readbuf[1]);
    if (readbuf[0] != 5) {
        return BadSocksVersionError();
    }
    if (readbuf[1] != 0) {
        return NoAvailableSocksAuthenticationError();
    }
    return true;
}

ErrorOr<Buffer> socks5_format_connect_request(Settings settings, Var<Logger> logger) {
    Buffer out;

    out.write_uint8(5); // Version
    out.write_uint8(1); // CMD_CONNECT
    out.write_uint8(0); // Reserved
    out.write_uint8(3); // ATYPE_DOMAINNAME

    logger->debug("socks5: >> version=5");
    logger->debug("socks5: >> CMD_CONNECT (1)");
    logger->debug("socks5: >> Reserved (0)");
    logger->debug("socks5: >> ATYPE_DOMAINNAME (3)");

    auto address = settings["net/address"];

    if (address.length() > 255) {
        return SocksAddressTooLongError();
    }
    out.write_uint8(address.length());            // Len
    out.write(address.c_str(), address.length()); // String

    logger->debug("socks5: >> domain len=%d", (uint8_t)address.length());
    logger->debug("socks5: >> domain str=%s", address.c_str());

    int portnum = settings["net/port"].as<int>();
    if (portnum < 0 || portnum > 65535) {
        return SocksInvalidPortError();
    }
    out.write_uint16(portnum); // Port

    logger->debug("socks5: >> port=%d", portnum);

    return out;
}

ErrorOr<bool> socks5_parse_connect_response(Buffer &buffer, Var<Logger> logger) {
    if (buffer.length() < 5) {
        return false; // Try again after next recv()
    }

    auto peekbuf = buffer.peek(5);

    logger->debug("socks5: << version=%d", peekbuf[0]);
    logger->debug("socks5: << reply=%d", peekbuf[1]);
    logger->debug("socks5: << reserved=%d", peekbuf[2]);
    logger->debug("socks5: << atype=%d", peekbuf[3]);

    if (peekbuf[0] != 5) {
        return BadSocksVersionError();
    }
    if (peekbuf[1] != 0) {
        return SocksError(); // TODO: also return the actual error
    }
    if (peekbuf[2] != 0) {
        return BadSocksReservedFieldError();
    }

    auto atype = peekbuf[3]; // Atype

    size_t total = 4; // Version .. Atype size
    if (atype == 1) {
        total += 4; // IPv4 addr size
    } else if (atype == 3) {
        total += 1 + peekbuf[4]; // Len size + String size
    } else if (atype == 4) {
        total += 16; // IPv6 addr size
    } else {
        return BadSocksAtypeValueError();
    }
    total += 2; // Port size
    if (buffer.length() < total) {
        return false; // Try again after next recv()
    }

    buffer.discard(total);
    return true;
}

void socks5_connect(std::string address, int port, Settings settings,
        Callback<Error, Var<Transport>> callback,
        Var<Reactor> reactor, Var<Logger> logger) {

    auto proxy = settings["net/socks5_proxy"];
    auto pos = proxy.find(":");
    if (pos == std::string::npos) {
        throw std::runtime_error("invalid argument");
    }
    auto proxy_address = proxy.substr(0, pos);
    auto proxy_port = proxy.substr(pos + 1);

    settings["net/address"] = address;
    settings["net/port"] = port;

    connect_logic(proxy_address, lexical_cast<int>(proxy_port),
            [=](Error err, Var<ConnectResult> r) {
                if (err) {
                    err.context = r;
                    callback(err, nullptr);
                    return;
                }
                Var<Transport> txp = Connection::make(r->connected_bev,
                                                      reactor, logger);
                Var<Transport> socks5(
                        new Socks5(txp, settings, reactor, logger));
                socks5->on_connect([=]() {
                    socks5->on_connect(nullptr);
                    socks5->on_error(nullptr);
                    Error error = NoError();
                    error.context = r;
                    callback(error, socks5);
                });
                socks5->on_error([=](Error error) {
                    socks5->on_connect(nullptr);
                    socks5->on_error(nullptr);
                    error.context = r;
                    callback(error, nullptr);
                });
            },
            settings, reactor, logger);
}

void Socks5::socks5_connect_() {
    // Step #1: send out preferred authentication methods

    logger->debug("socks5: connected to Tor!");
    conn->write(socks5_format_auth_request(logger));

    // Step #2: receive the allowed authentication methods

    conn->on_data([this](Buffer d) {
        buffer << d;
        ErrorOr<bool> result = socks5_parse_auth_response(buffer, logger);
        if (!result) {
            emit_error(result.as_error());
            return;
        }
        if (!*result) {
            return;
        }

        // Step #3: ask Tor to connect to remote host

        ErrorOr<Buffer> out = socks5_format_connect_request(settings, logger);
        if (!out) {
            emit_error(out.as_error());
            return;
        }
        conn->write(*out);

        // Step #4: receive Tor's response

        conn->on_data([this](Buffer d) {
            buffer << d;
            ErrorOr<bool> rc = socks5_parse_connect_response(buffer, logger);
            if (!rc) {
                emit_error(rc.as_error());
                return;
            }
            if (!*rc) {
                return;
            }

            //
            // Step #5: we are now connected
            // Restore the original hooks
            // Tell upstream we are connected
            // If more data, pass it up
            //

            conn->on_data([this](Buffer d) { emit_data(d); });
            conn->on_flush([this]() { emit_flush(); });

            emit_connect();

            // Note that emit_connect() may have called close()
            if (!isclosed && buffer.length() > 0) {
                emit_data(buffer);
            }
        });
    });
}

} // namespace net
} // namespace mk
