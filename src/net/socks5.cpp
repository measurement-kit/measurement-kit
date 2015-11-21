// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/net/socks5.hpp"

namespace measurement_kit {
namespace net {

Socks5::Socks5(Settings s, Logger *lp)
    : Dumb(lp), settings(s),
      conn(settings["family"].c_str(), settings["socks5_address"].c_str(),
           settings["socks5_port"].c_str()),
      proxy_address(settings["socks5_address"]),
      proxy_port(settings["socks5_port"]) {

    logger->debug("socks5: connecting to Tor at %s:%s",
                  settings["socks5_address"].c_str(),
                  settings["socks5_port"].c_str());

    // Step #0: Steal "error", "connect", and "flush" handlers

    conn.on_error([this](Error err) { emit_error(err); });
    conn.on_connect([this]() {
        conn.on_flush([]() {
            // Nothing
        });

        // Step #1: send out preferred authentication methods

        logger->debug("socks5: connected to Tor!");

        Buffer out;
        out.write_uint8(5); // Version
        out.write_uint8(1); // Number of methods
        out.write_uint8(0); // "NO_AUTH" meth.
        conn.send(out);

        logger->debug("socks5: >> version=5");
        logger->debug("socks5: >> number of methods=1");
        logger->debug("socks5: >> NO_AUTH (0)");

        // Step #2: receive the allowed authentication methods

        conn.on_data([this](Buffer &d) {
            buffer << d;
            auto readbuf = buffer.readn(2);
            if (readbuf == "") {
                return; // Try again after next recv()
            }

            logger->debug("socks5: << version=%d", readbuf[0]);
            logger->debug("socks5: << auth=%d", readbuf[1]);

            if (readbuf[0] != 5 || // Reply version
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

            logger->debug("socks5: >> domain len=%d",
                          (uint8_t)address.length());
            logger->debug("socks5: >> domain str=%s", address.c_str());

            auto portnum = std::stoi(settings["port"]);
            if (portnum < 0 || portnum > 65535) {
                emit_error(SocksInvalidPortError());
                return;
            }
            out.write_uint16(portnum); // Port

            logger->debug("socks5: >> port=%d", portnum);

            conn.send(out);

            // Step #4: receive Tor's response

            conn.on_data([this](Buffer &d) {

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

                if (peekbuf[0] != 5 || // Version
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

                conn.on_data([this](Buffer &d) { emit_data(d); });
                conn.on_flush([this]() { emit_flush(); });

                emit_connect();

                // Note that emit_connect() may have called close()
                if (!isclosed && buffer.length() > 0) {
                    emit_data(buffer);
                }
            });
        });
    });
}
}
}
