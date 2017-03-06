// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../cmdline.hpp"

namespace meek_fronted_requests {

#define USAGE                                                  \
    "usage: measurement_kit [options] meek_fronted_requests\n" \
    "                       [-B expected_body] input_file\n"

int main(std::list<Callback<BaseTest &>> &initializers, int argc, char **argv) {
    Settings settings;
    std::string name = argv[0];
    uint32_t verbosity = 0;
    std::string expected_body;
    mk::nettests::MeekFrontedRequestsTest test;
    int ch;
    while ((ch = getopt(argc, argv, "B:nv")) != -1) {
        switch (ch) {
        case 'B':
            test.set_options("expected_body", optarg);
            break;
        case 'n':
            test.set_options("no_collector", true);
            break;
        case 'v':
            ++verbosity;
            break;
        default:
            fprintf(stderr, "%s\n", USAGE);
            exit(1);
        }
    }
    argc -= optind;
    argv += optind;

    if (argc < 1) {
        fprintf(stderr, "%s\n", USAGE);
        exit(1);
        /* NOTREACHED */
    }
    while (argc > 0) {
        test.add_input_filepath(argv[0]);
        argc -= 1, argv += 1;
    }

    common_init(initializers, test).run();
    return 0;
}

} // namespace meek_fronted_requests
