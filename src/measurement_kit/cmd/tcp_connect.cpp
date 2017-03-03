// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../cmdline.hpp"

namespace tcp_connect {

#define USAGE                                                                  \
    "usage: measurement_kit [options] tcp_connect [-f input_file] [-p port]"

int main(std::list<Callback<BaseTest &>> &initializers, int argc, char **argv) {
    mk::nettests::TcpConnectTest test;
    std::string port = "80";

    for (int ch; (ch = getopt(argc, argv, "f:p:")) != -1;) {
        switch (ch) {
        case 'f':
            test.add_input_filepath(optarg);
            break;
        case 'p':
            port = optarg;
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

    common_init(initializers, test.set_options("port", port)).run();
    return 0;
}

} // namespace tcp_connect
