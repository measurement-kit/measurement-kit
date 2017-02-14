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
    mk::nettests::MeekFrontingTest test;
    int ch;
    while ((ch = mkp_getopt(argc, argv, "v")) != -1) {
        switch (ch) {
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
    while (argc > 0) {
        test.add_input_filepath(argv[0]);
        argc -= 1, argv += 1;
    }

    test
        .set_verbosity(verbosity)
        .on_log([](uint32_t, const char *s) { std::cout << s << "\n"; })
        .run();

    return 0;
}

} // namespace meek_fronting
} // namespace cmdline
} // namespace mk
