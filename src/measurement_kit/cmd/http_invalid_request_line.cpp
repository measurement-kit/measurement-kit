// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../cmdline.hpp"

namespace http_invalid_request_line {

#define USAGE                                                                  \
    "usage: measurement_kit [options] http_invalid_request_line\n"             \
    "                       [-b backend_hostname] [-p port]"

int main(std::list<Callback<BaseTest &>> &initializers, int argc, char **argv) {
    std::string backend_hostname = "a.echo.th.ooni.io";
    std::string backend_port = "80";
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

    std::string backend = "http://" + backend_hostname + ":" + backend_port;
    common_init(initializers, test.set_options("backend", backend)).run();
    return 0;
}

} // namespace http_invalid_request_line
