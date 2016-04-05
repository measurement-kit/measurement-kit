// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

//
// This example shows how to use net:connect()
// FIXME: refactor this to use a public api when Transport will be changed
//

#include "src/net/connect.hpp"
#include <functional>
#include <iostream>
#include <measurement_kit/common.hpp>
#include <measurement_kit/net.hpp>
#include <stdlib.h>
#include <string>
#include <unistd.h>

using namespace mk;
using namespace mk::net;

static const char *kv_usage =
        "usage: ./example/net/connect [-v] [-p port] domain\n";

int main(int argc, char **argv) {

    int port = 80;
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
    argc -= optind;
    argv += optind;

    if (argc != 1) {
        std::cout << kv_usage;
        exit(1);
    }
    std::string domain = argv[0];

    loop_with_initial_event([&domain, &port]() {
        connect(domain, port, [](ConnectResult r) {
            if (!r.overall_error) {
                std::cout << "Connection successful!\n";
                ::bufferevent_free(r.connected_bev);
            }
            break_loop();
        });
    });
}
