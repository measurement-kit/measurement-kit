// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../cmdline.hpp"

namespace web_connectivity {

#define USAGE                                                                  \
    "usage: measurement_kit [options] web_connectivity [-b backend]\n"         \
    "                       [-f input_file] [-t timeout]\n"

int main(std::list<Callback<BaseTest &>> &initializers, int argc, char **argv) {
    mk::nettests::WebConnectivityTest test;

    for (int ch; (ch = getopt(argc, argv, "b:f:t:")) != -1;) {
        switch (ch) {
        case 'b':
            test.set_options("backend", optarg);
            break;
        case 'f':
            test.add_input_filepath(optarg);
            break;
        case 't':
            test.set_options("max_runtime", optarg);
            break;
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

} // namespace web_connectivity
