// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

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
        "usage: ./example/net/connect [-Tv] [-p port] domain\n";

int main(int argc, char **argv) {

    int port = 80;
    Settings settings;
    char ch;
    while ((ch = getopt(argc, argv, "p:Tv")) != -1) {
        switch (ch) {
        case 'p':
            port = lexical_cast<int>(optarg);
            break;
        case 'T':
            settings["socks5_proxy"] = "127.0.0.1:9050";
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
    if (argc != 1) {
        std::cout << kv_usage;
        exit(1);
    }
    std::string domain = argv[0];

    loop_with_initial_event([&domain, &port, &settings]() {
        connect(domain, port, [](Error err, Var<Transport>) {
            std::cout << "Connection result code: " << (int)err << "\n";
            break_loop();
        }, settings);
    });
}
