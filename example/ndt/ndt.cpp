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
using namespace mk::ndt;

static const char *kv_usage =
    "usage: ./example/net/ndt [-C /path/to/ca/bundle] [-v] [-p port] [host]\n";

int main(int argc, char **argv) {

    NdtTest test;
    char ch;
    while ((ch = getopt(argc, argv, "C:p:v")) != -1) {
        switch (ch) {
        case 'C':
            test.set_options("net/ca_bundle_path", optarg);
            break;
        case 'p':
            test.set_options("port", optarg);
            break;
        case 'v':
            test.increase_verbosity();
            break;
        default:
            std::cout << kv_usage;
            exit(1);
        }
    }
    argc -= optind;
    argv += optind;

    if (argc > 1) {
        std::cout << kv_usage;
        exit(1);
    } else if (argc == 1) {
        test.set_options("address", argv[0]);
    }

    test.run();
}
