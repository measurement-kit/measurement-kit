/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#include <ight/net/socks5.hpp>

using namespace ight::net::connection;
using namespace ight::net::socks5;

Socks5::Socks5(Settings s) : settings(s) {

    ight_debug("socks5: connecting to Tor at %s:%s",
               settings["socks5_address"].c_str(),
               settings["socks5_port"].c_str());

    // Save address and port so they can be accessed later
    proxy_address = settings["socks5_address"];
    proxy_port = settings["socks5_port"];

    conn = std::make_shared<Connection>(settings["family"].c_str(),
                                        settings["socks5_address"].c_str(),
                                        settings["socks5_port"].c_str());

    // Step #0: Steal "connect" and "flush" handlers

    conn->on_connect([this]() {
        conn->on_flush([]() {
            // Nothing
        });

        // Step #1: send out preferred authentication methods

        ight_debug("socks5: connected to Tor!");

        Buffer out;
        out.write_uint8(5); // Version
        out.write_uint8(1); // Number of methods
        out.write_uint8(0); // "NO_AUTH" meth.
        conn->send(out);

        ight_debug("socks5: >> version=5");
        ight_debug("socks5: >> number of methods=1");
        ight_debug("socks5: >> NO_AUTH (0)");

        // Step #2: receive the allowed authentication methods

        conn->on_data([this](SharedPointer<Buffer> d) {
            *buffer << *d;
            auto readbuf = buffer->readn(2);
            if (readbuf == "") {
                return; // Try again after next recv()
            }

            ight_debug("socks5: << version=%d", readbuf[0]);
            ight_debug("socks5: << auth=%d", readbuf[1]);

            if (readbuf[0] != 5 || // Reply version
                readbuf[1] != 0) { // Preferred auth method
                throw std::runtime_error("generic error");
            }

            // Step #3: ask Tor to connect to remote host

            Buffer out;
            out.write_uint8(5); // Version
            out.write_uint8(1); // CMD_CONNECT
            out.write_uint8(0); // Reserved
            out.write_uint8(3); // ATYPE_DOMAINNAME

            ight_debug("socks5: >> version=5");
            ight_debug("socks5: >> CMD_CONNECT (0)");
            ight_debug("socks5: >> Reserved (0)");
            ight_debug("socks5: >> ATYPE_DOMAINNAME (3)");

            auto address = settings["address"];

            if (address.length() > 255) {
                throw std::runtime_error("generic error");
            }
            out.write_uint8(address.length());            // Len
            out.write(address.c_str(), address.length()); // String

            ight_debug("socks5: >> domain len=%d", (uint8_t)address.length());
            ight_debug("socks5: >> domain str=%s", address.c_str());

            auto portnum = std::stoi(settings["port"]);
            if (portnum < 0 || portnum > 65535) {
                throw std::runtime_error("generic error");
            }
            out.write_uint16(portnum); // Port

            ight_debug("socks5: >> port=%d", portnum);

            conn->send(out);

            // Step #4: receive Tor's response

            conn->on_data([this](SharedPointer<Buffer> d) {

                *buffer << *d;
                if (buffer->length() < 5) {
                    return; // Try again after next recv()
                }

                auto peekbuf = buffer->peek(5);

                ight_debug("socks5: << version=%d", peekbuf[0]);
                ight_debug("socks5: << reply=%d", peekbuf[1]);
                ight_debug("socks5: << reserved=%d", peekbuf[2]);
                ight_debug("socks5: << atype=%d", peekbuf[3]);

                // TODO: Here we should process peekbuf[1] more
                // carefully to map to the error that occurred
                // and report it correctly to the caller

                if (peekbuf[0] != 5 || // Version
                    peekbuf[1] != 0 || // Reply
                    peekbuf[2] != 0) { // Reserved
                    throw std::runtime_error("generic error");
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
                    throw std::runtime_error("generic error");
                }
                total += 2; // Port size
                if (buffer->length() < total) {
                    return; // Try again after next recv()
                }

                buffer->discard(total);

                //
                // Step #5: we are now connected
                // Restore the original hooks
                // Tell upstream we are connected
                // If more data, pass it up
                //

                conn->on_data(
                    [this](SharedPointer<Buffer> d) { on_data_fn(d); });
                conn->on_flush([this]() { on_flush_fn(); });

                on_connect_fn();

                // Note that on_connect_fn() may have called close()
                if (!isclosed && buffer->length() > 0) {
                    on_data_fn(buffer);
                }
            });
        });
    });
}
