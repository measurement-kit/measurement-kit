// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "../cmdline.hpp"

namespace captiveportal {

#define USAGE "usage: measurement_kit [options] captiveportal\n"

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

} // namespace captiveportal
