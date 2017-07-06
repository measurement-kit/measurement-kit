// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../cmdline.hpp"

namespace captive_portal {

#define USAGE                                                                  \
    "usage: measurement_kit [options] captive_portal [-b backend]\n"           \
    "                       [-f input_file] [-t timeout]\n"

int main(std::list<Callback<BaseTest &>> &initializers, int argc, char **argv) {
    mk::nettests::CaptivePortalTest test;

    for (int ch; (ch = getopt(argc, argv, "")) != -1;) {
        switch (ch) {
        default:
            fprintf(stderr, "%s\n", USAGE);
            exit(1);
            /* NOTREACHED */
        }
    }
    argc -= optind, argv += optind;
    if (argc != 0) {
        fprintf(stderr, "%s\n", USAGE);
        exit(1);
        /* NOTREACHED */
    }

    common_init(initializers, test).run();
    return 0;
}

} // namespace captive_portal
