// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <functional>
#include <iostream>
#include <measurement_kit/common/error.hpp>
#include <measurement_kit/common/poller.hpp>
#include <measurement_kit/common/var.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <vector>
#include "src/net/bev-connect.hpp"

// Forward declarations
struct bufferevent;

using namespace mk;

#define USAGE "usage: %s [-Sv] [-p port] address [...]\n"

int main(int argc, char **argv) {
    std::string port = "80";
    const char *progname = argv[0];
    bool use_ssl = false;
    int ch;

    while ((ch = getopt(argc, argv, "p:Sv")) != -1) {
        switch (ch) {
        case 'p':
            port = optarg;
            break;
        case 'S':
            use_ssl = true;
            break;
        case 'v':
            mk::set_verbose(1);
            break;
        default:
            fprintf(stderr, USAGE, progname);
            exit(1);
        }
    }
    argc -= optind, argv += optind;
    std::vector<std::string> endpoints;
    while (argc > 0) {
        endpoints.push_back(std::string(argv[0]) + ":" + port);
        --argc, ++argv;
    }
    if (endpoints.size() == 0) {
        fprintf(stderr, USAGE, progname);
        exit(1);
    }

    Poller *poller = Poller::global();
    poller->call_soon([endpoints, poller, use_ssl]() {
        net::connect_one_of(
            endpoints, [poller, use_ssl](Error error, Var<bufferevent> bev) {
                if (error || !use_ssl) {
                    poller->break_loop();
                    return;
                }
                net::connect_ssl(bev, [poller](Error, Var<bufferevent>) {
                    poller->break_loop();
                }, 3.0, poller);
            }, 3.0, poller);
    });
    poller->loop();
}
