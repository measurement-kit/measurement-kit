// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "../cmdline.hpp"

namespace web_connectivity {

#define USAGE                                                                  \
    "usage: measurement_kit [options] web_connectivity [-b backend]\n"         \
    "                       [-f input_file] [-t timeout] [-u url]\n"

int main(std::list<Callback<BaseTest &>> &initializers, int argc, char **argv) {
    mk::nettests::WebConnectivityTest test;

    for (int ch; (ch = getopt(argc, argv, "b:f:t:u:")) != -1;) {
        switch (ch) {
        case 'b':
            test.set_option("backend", optarg);
            break;
        case 'f':
            test.add_input_filepath(optarg);
            break;
        case 't':
            test.set_option("max_runtime", optarg);
            break;
        case 'u':
            test.add_input(optarg);
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
