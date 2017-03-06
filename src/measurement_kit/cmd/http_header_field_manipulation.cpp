// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../cmdline.hpp"

namespace http_header_field_manipulation {

#define USAGE "usage: measurement_kit http_header_field_manipulation [-v]\n"

int main(std::list<Callback<BaseTest &>> &initializers, int argc, char **argv) {
    std::string backend = "http://38.107.216.10:80";
    mk::nettests::HttpHeaderFieldManipulationTest test;
    int ch;
    while ((ch = getopt(argc, argv, "b:")) != -1) {
        switch (ch) {
        case 'b':
            backend = optarg;
            break;
        default:
            fprintf(stderr, "%s\n", USAGE);
            exit(1);
        }
    }
    argc -= optind;
    argv += optind;

    common_init(initializers, test.set_options("backend", backend)).run();
    return 0;
}

} // namespace http_header_field_manipulation
