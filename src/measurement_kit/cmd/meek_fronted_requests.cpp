// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "../cmdline.hpp"

namespace meek_fronted_requests {

#define USAGE                                                    \
    "usage: measurement_kit [options] meek_fronted_requests\n"   \
    "                       [-B expected_body] -f input_file\n"

int main(std::list<Callback<BaseTest &>> &initializers, int argc, char **argv) {
    mk::nettests::MeekFrontedRequestsTest test;
    int ch;
    while ((ch = getopt(argc, argv, "B:f:")) != -1) {
        switch (ch) {
        case 'B':
            test.set_option("expected_body", optarg);
            break;
        case 'f':
            test.add_input_filepath(optarg);
            break;
        default:
            fprintf(stderr, "%s\n", USAGE);
            exit(1);
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

} // namespace meek_fronted_requests
