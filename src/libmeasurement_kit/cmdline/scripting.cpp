// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/cmdline.hpp>
#include <measurement_kit/scripting.hpp>

#include <iostream>

namespace mk {
namespace cmdline {
namespace scripting {

static const char *kv_usage = "usage: measurement_kit scripting [-v] script\n";

int main(const char *, int argc, char **argv) {

    for (int ch; (ch = mkp_getopt(argc, argv, "v")) != -1; ) {
        switch (ch) {
        case 'v':
            increase_verbosity();
            break;
        default:
            std::cout << kv_usage;
            exit(1);
        }
    }
    argc -= mkp_optind, argv += mkp_optind;
    if (argc != 1) {
        std::cout << kv_usage;
        exit(1);
    }
    mk::scripting::run(argv[0]);
    return 0;
}

} // namespace scripting
} // namespace cmdline
} // namespace mk
