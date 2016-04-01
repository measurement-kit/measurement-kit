// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/net/filter.hpp"
#include "src/net/listen.hpp"
#include <functional>
#include <iostream>
#include <measurement_kit/common.hpp>
#include <measurement_kit/net.hpp>
#include <stdlib.h>
#include <string>
#include <unistd.h>

using namespace mk;
using namespace mk::net;

static const char *kv_usage = "usage: ./example/net/filter [-v] [-p port]\n";

int main(int argc, char **argv) {

    int port = 54321;
    char ch;
    while ((ch = getopt(argc, argv, "p:v")) != -1) {
        switch (ch) {
        case 'p':
            port = lexical_cast<int>(optarg);
            break;
        case 'v':
            set_verbose(1);
            break;
        default:
            std::cout << kv_usage;
            exit(1);
        }
    }
    argc -= optind, argv += optind;
    if (argc != 0) {
        std::cout << kv_usage;
        exit(1);
    }
    const std::string addr = "127.0.0.1";

    loop_with_initial_event([&addr, &port]() {
        listen4(addr, port, [](bufferevent *orig) {
            filter_bufferevent<Filter>(orig, [](Error error, Filter *filter) {
                if (error) {
                    return;
                }

                // Setup filter to make input uppercase
                filter->on_input([](evbuffer *src, evbuffer *dst, ev_ssize_t,
                        bufferevent_flush_mode) {
                    std::string out;
                    for (auto chr : Buffer(src).read()) {
                        out += toupper(chr);
                    }
                    Buffer obuf = out;
                    obuf >> dst;
                    return BEV_OK;
                });

                // Auto-removing output filter to write banner
                filter->on_output([filter](evbuffer *, evbuffer *dst,
                        ev_ssize_t, bufferevent_flush_mode) {
                    Buffer buf("THIS ECHO SERVER MAKES INPUT UPPERCASE\r\n");
                    buf >> dst;
                    filter->on_output(nullptr);
                    return BEV_OK;
                });

                // This is the standard code to echo input back
                Var<Transport> transport(new Connection(filter->bev));
                transport->on_data(
                        [transport](Buffer data) { transport->write(data); });
                transport->on_error([transport](Error) {
                    transport->on_error(nullptr);
                    transport->on_data(nullptr);
                    // FIXME: if we close now in response to EOF not
                    // all output is written in all cases
                    transport->close();
                });

                // Immediately trigger filter output to write banner
                filter->trigger_output();
            });
        });
    });
}
