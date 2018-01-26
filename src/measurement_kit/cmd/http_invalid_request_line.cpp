// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "../cmdline.hpp"

namespace http_invalid_request_line {

#define USAGE                                                                  \
    "usage: measurement_kit [options] http_invalid_request_line\n"             \
    "                       [-b backend_hostname] [-p port]"

int main(std::list<Callback<BaseTest &>> &initializers, int argc, char **argv) {
    std::string backend_hostname;
    std::string backend_port;
    mk::nettests::HttpInvalidRequestLineTest test;

    for (int ch; (ch = getopt(argc, argv, "b:p:")) != -1;) {
        switch (ch) {
        case 'b':
            backend_hostname = optarg;
            break;
        case 'p':
            backend_port = optarg;
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

    if (backend_hostname != "") {
        std::string backend = backend_hostname;
        if (backend_port != "") {
            backend += ":";
            backend += backend_port;
        }
        test.set_option("backend", backend);
    }
    common_init(initializers, test).run();
    return 0;
}

} // namespace http_invalid_request_line
