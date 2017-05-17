// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../cmdline.hpp"

namespace dns_injection {

#define USAGE                                                                  \
    "usage: measurement_kit [options] dns_injection [-f input_file]\n"         \
    "                       [-r invalid_resolver_ip]"

int main(std::list<Callback<BaseTest &>> &initializers, int argc, char **argv) {
    std::string backend = "8.8.8.1";
    mk::nettests::DnsInjectionTest test;

    for (int ch; (ch = getopt(argc, argv, "f:r:")) != -1;) {
        switch (ch) {
        case 'f':
            test.add_input_filepath(optarg);
            break;
        case 'r':
            backend = optarg;
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

    common_init(initializers, test.set_options("backend", backend)).run();
    return 0;
}

} // namespace dns_injection
