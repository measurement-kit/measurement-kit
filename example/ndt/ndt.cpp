// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <functional>
#include <iostream>
#include <measurement_kit/common.hpp>
#include <measurement_kit/ndt.hpp>
#include <stdlib.h>
#include <string>
#include <unistd.h>

using namespace mk;

static const char *kv_usage = "usage: ./example/net/ndt [-v] [-p port] host\n";

int main(int argc, char **argv) {

    int port = 3001;
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
    argc -= optind;
    argv += optind;

    if (argc != 1) {
        std::cout << kv_usage;
        exit(1);
    }
    std::string host = argv[0];

    loop_with_initial_event([&]() {
        ndt::client(host, port, [](Error err) {
            std::cout << "result: " << err << "\n";
            break_loop();
        });
    });
}
