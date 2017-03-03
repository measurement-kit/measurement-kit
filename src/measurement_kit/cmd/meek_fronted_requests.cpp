// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../cmdline.hpp"

#include <iostream>

namespace meek_fronted_requests {

static const char *kv_usage =
    "usage: measurement_kit meek_fronted_requests [-v] input_file \n";

int main(std::list<Callback<BaseTest &>> &initializers, int argc, char **argv) {
    Settings settings;
    std::string name = argv[0];
    uint32_t verbosity = 0;
    std::string expected_body, outer_host, inner_host;
    mk::nettests::MeekFrontedRequestsTest test;
    int ch;
    while ((ch = getopt(argc, argv, "B:D:H:nv")) != -1) {
        switch (ch) {
        case 'B':
            test.set_options("expected_body", optarg);
            break;
        case 'D':
            test.set_options("outer_host", optarg);
            break;
        case 'H':
            test.set_options("inner_host", optarg);
            break;
        case 'n':
            test.set_options("no_collector", true);
            break;
        case 'v':
            ++verbosity;
            break;
        default:
            std::cout << kv_usage;
            exit(1);
        }
    }
    argc -= optind;
    argv += optind;

    if ((inner_host.empty() && !outer_host.empty()) ||
        (outer_host.empty() && !inner_host.empty())) {
        std::cout << "If you specify one of {outer,inner}_host, "
                     "you must specify both.\n";
        std::cout << kv_usage;
        exit(1);
        /* NOTREACHED */
    }

    if (inner_host.empty() && outer_host.empty()) {
        test.runnable->needs_input = true;
        if (argc < 1) {
            std::cout << kv_usage;
            exit(1);
            /* NOTREACHED */
        }
        while (argc > 0) {
            test.add_input_filepath(argv[0]);
            argc -= 1, argv += 1;
        }
    }

    common_init(initializers, test).run();
    return 0;
}

} // namespace meek_fronted_requests
