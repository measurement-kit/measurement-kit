// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../cmdline.hpp"

namespace dash {

#define USAGE "usage: measurement_kit [options] dash [-U uuid] [hostname]"

int main(std::list<Callback<BaseTest &>> &initializers, int argc, char **argv) {
    nettests::DashTest test;
    for (int ch; (ch = getopt(argc, argv, "U:")) != -1;) {
        switch (ch) {
        case 'U':
            test.set_options("uuid", optarg);
            break;
        default:
            fprintf(stderr, "%s\n", USAGE);
            exit(1);
            /* NOTREACHED */
        }
    }
    argc -= optind, argv += optind;
    if (argc > 1) {
        fprintf(stderr, "%s\n", USAGE);
        exit(1);
        /* NOTREACHED */
    }
    if (argc == 1) {
        test.set_options("hostname", argv[0]);
    }

    common_init(initializers, test).run();
    return 0;
}

} // namespace dash
