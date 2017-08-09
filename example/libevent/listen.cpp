// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "private/libevent/listen.hpp"
#include <functional>
#include <iostream>
#include <measurement_kit/common.hpp>
#include <measurement_kit/net.hpp>
#include <stdlib.h>
#include <string>
#include <unistd.h>

using namespace mk;
using namespace mk::libevent;
using namespace mk::net;

static const char *kv_usage = "usage: ./example/libevent/listen [-v] [-p port]\n";

int main(int argc, char **argv) {

    int port = 54321;
    char ch;
    while ((ch = getopt(argc, argv, "p:v")) != -1) {
        switch (ch) {
        case 'p':
            port = lexical_cast<int>(optarg);
            break;
        case 'v':
            increase_verbosity();
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
        listen4(addr, port, [](bufferevent *bev) {
            Var<Transport> transport = Connection::make(bev,
                Reactor::global(), Logger::global());
            transport->on_data([transport](Buffer data) {
                transport->write(data);
            });
            transport->on_error([transport](Error) {
                transport->on_error(nullptr);
                transport->on_data(nullptr);
                transport->close([](){});
            });
        });
    });
}
