// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/net/socks5.hpp"
#include "src/net/connect.hpp"

namespace mk {
namespace net {

Socks5::Socks5(Settings s, Logger *lp, Poller *poller)
    : Emitter(lp), settings(s), proxy_address(settings["socks5_address"]),
      proxy_port(settings["socks5_port"]) {

    conn.reset(new Connection(settings["family"].c_str(),
            settings["socks5_address"].c_str(), settings["socks5_port"].c_str(),
            lp, poller));

    logger->debug("socks5: connecting to Tor at %s:%s",
            settings["socks5_address"].c_str(),
            settings["socks5_port"].c_str());

    // Step #0: Steal "error", "connect", and "flush" handlers

    conn->on_error([this](Error err) { emit_error(err); });
    conn->on_connect([this]() {
        logger->debug("socks5: connected to Tor!");
        conn->on_error(nullptr);
        conn->on_connect(nullptr);
        socks5_connect_();
    });
}

Socks5::Socks5(Var<Transport> tx, Settings s, Poller *, Logger *lp)
    : Emitter(lp), settings(s), conn(tx),
      proxy_address(settings["socks5_address"]),
      proxy_port(settings["socks5_port"]) {
    socks5_connect_();
}

void Socks5::socks5_connect_() {
    // Step #1: send out preferred authentication methods

    conn->on_error([this](Error err) { emit_error(err); });

    Buffer out;
    out.write_uint8(5); // Version
    out.write_uint8(1); // Number of methods
    out.write_uint8(0); // "NO_AUTH" meth.
    conn->send(out);

    logger->debug("socks5: >> version=5");
    logger->debug("socks5: >> number of methods=1");
    logger->debug("socks5: >> NO_AUTH (0)");

    // Step #2: receive the allowed authentication methods

    conn->on_data([this](Buffer d) {
        buffer << d;
        auto readbuf = buffer.readn(2);
        if (readbuf == "") {
            return; // Try again after next recv()
        }

        logger->debug("socks5: << version=%d", readbuf[0]);
        logger->debug("socks5: << auth=%d", readbuf[1]);

        if (readbuf[0] != 5 ||     // Reply version
                readbuf[1] != 0) { // Preferred auth method
            emit_error(BadSocksVersionError());
            return;
        }

        // Step #3: ask Tor to connect to remote host

        Buffer out;
        out.write_uint8(5); // Version
        out.write_uint8(1); // CMD_CONNECT
        out.write_uint8(0); // Reserved
        out.write_uint8(3); // ATYPE_DOMAINNAME

        logger->debug("socks5: >> version=5");
        logger->debug("socks5: >> CMD_CONNECT (0)");
        logger->debug("socks5: >> Reserved (0)");
        logger->debug("socks5: >> ATYPE_DOMAINNAME (3)");

        auto address = settings["address"];

        if (address.length() > 255) {
            emit_error(SocksAddressTooLongError());
            return;
        }
        out.write_uint8(address.length());            // Len
        out.write(address.c_str(), address.length()); // String

        logger->debug("socks5: >> domain len=%d", (uint8_t)address.length());
        logger->debug("socks5: >> domain str=%s", address.c_str());

        int portnum = settings["port"].as<int>();
        if (portnum < 0 || portnum > 65535) {
            emit_error(SocksInvalidPortError());
            return;
        }
        out.write_uint16(portnum); // Port

        logger->debug("socks5: >> port=%d", portnum);

        conn->send(out);

        // Step #4: receive Tor's response

        conn->on_data([this](Buffer d) {

            buffer << d;
            if (buffer.length() < 5) {
                return; // Try again after next recv()
            }

            auto peekbuf = buffer.peek(5);

            logger->debug("socks5: << version=%d", peekbuf[0]);
            logger->debug("socks5: << reply=%d", peekbuf[1]);
            logger->debug("socks5: << reserved=%d", peekbuf[2]);
            logger->debug("socks5: << atype=%d", peekbuf[3]);

            // TODO: Here we should process peekbuf[1] more
            // carefully to map to the error that occurred
            // and report it correctly to the caller

            if (peekbuf[0] != 5 ||     // Version
                    peekbuf[1] != 0 || // Reply
                    peekbuf[2] != 0) { // Reserved
                emit_error(SocksGenericError());
                return;
            }
            auto atype = peekbuf[3]; // Atype

            size_t total = 4; // Version .. Atype size
            if (atype == 1) {
                total += 4; // IPv4 addr size
            } else if (atype == 3) {
                total += 1             // Len size
                         + peekbuf[4]; // String size
            } else if (atype == 4) {
                total += 16; // IPv6 addr size
            } else {
                emit_error(SocksGenericError());
                return;
            }
            total += 2; // Port size
            if (buffer.length() < total) {
                return; // Try again after next recv()
            }

            buffer.discard(total);

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

void socks5_connect(std::string address, int port, Settings settings,
        std::function<void(Error, Var<Transport>)> callback,
        Poller *poller, Logger *logger) {

    auto proxy = settings["socks5_proxy"];
    auto pos = proxy.find(":");
    if (pos == std::string::npos) {
        throw std::runtime_error("invalid argument");
    }
    auto proxy_address = proxy.substr(0, pos);
    auto proxy_port = proxy.substr(pos + 1);

    settings["address"] = address;
    settings["port"] = port;

    connect(proxy_address, lexical_cast<int>(proxy_port),
            [=](ConnectResult r) {
                if (r.overall_error) {
                    callback(r.overall_error, nullptr);
                    return;
                }
                Var<Transport> txp(new Connection(r.connected_bev));
                Var<Transport> socks5(
                        new Socks5(txp, settings, poller, logger));
                socks5->on_connect([=]() {
                    socks5->on_connect(nullptr);
                    socks5->on_error(nullptr);
                    callback(NoError(), socks5);
                });
                socks5->on_error([=](Error error) {
                    socks5->on_connect(nullptr);
                    socks5->on_error(nullptr);
                    callback(error, nullptr);
                });
            },
            settings.get("timeo", 10.0), poller, logger);
}

} // namespace net
} // namespace mk
