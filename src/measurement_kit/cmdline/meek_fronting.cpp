// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../cmdline/cmdline.hpp"
#include <measurement_kit/nettests.hpp>
#include <measurement_kit/http.hpp>

#include <iostream>

namespace mk {
namespace cmdline {
namespace meek_fronting {

static const char *kv_usage =
    "usage: measurement_kit meek_fronting [-v] input_file \n";

int main(const char *, int argc, char **argv) {
    Settings settings;
    std::string name = argv[0];
    uint32_t verbosity = 0;
    std::string expected_body, outer_host, inner_host;
    mk::nettests::MeekFrontingTest test;
    int ch;
    while ((ch = mkp_getopt(argc, argv, "B:D:H:nv")) != -1) {
        switch (ch) {
        case 'B':
            expected_body = mkp_optarg;
            break;
        case 'D':
            outer_host = mkp_optarg;
            break;
        case 'H':
            inner_host = mkp_optarg;
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
    argc -= mkp_optind;
    argv += mkp_optind;

    if (inner_host.empty()) {
        std::cout << "inner_host empty\n";
    }
    if (outer_host.empty()) {
        std::cout << "outer_host empty\n";
    }

    if ((inner_host.empty() && !outer_host.empty()) ||
        (outer_host.empty() && !inner_host.empty())) {
        std::cout << "If you specify one of {outer,inner}_host, "
                     "you must specify both.\n";
        std::cout << kv_usage;
        exit(1);
    }

    if (inner_host.empty() && outer_host.empty()) {
        if (argc < 1) {
            std::cout << kv_usage;
            exit(1);
        }
        while (argc > 0) {
            test.add_input_filepath(argv[0]);
            argc -= 1, argv += 1;
        }
    }

    test
        .set_options("expected_body", expected_body)
        .set_options("outer_host", outer_host)
        .set_options("inner_host", inner_host)
        .set_verbosity(verbosity)
        .on_log([](uint32_t, const char *s) { std::cout << s << "\n"; })
        .run();

    return 0;
}

} // namespace meek_fronting
} // namespace cmdline
} // namespace mk
