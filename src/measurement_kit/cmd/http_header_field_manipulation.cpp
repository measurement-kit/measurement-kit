// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../cmdline.hpp"

namespace http_header_field_manipulation {

#define USAGE "usage: measurement_kit [options] http_header_field_manipulation [-b backend]\n"

int main(std::list<Callback<BaseTest &>> &initializers, int argc, char **argv) {
    std::string backend = "http://a.http.th.ooni.io";
    mk::nettests::HttpHeaderFieldManipulationTest test;
    for (int ch; (ch = getopt(argc, argv, "b:")) != -1; ) {
        switch (ch) {
        case 'b':
            backend = optarg;
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

    common_init(initializers, test.set_options("backend", backend)).run();
    return 0;
}

} // namespace http_header_field_manipulation
