// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

//
// This example will run a synchronous http_invalid_request_line test
//

#include <measurement_kit/ooni.hpp>
#include <iostream>
#include <string>
#include <unistd.h>

int main(int argc, char **argv) {
    std::string backend = "http://213.138.109.232/";
    uint32_t verbosity = 0;
    std::string name = argv[0];
    char ch;

    while ((ch = getopt(argc, argv, "b:v")) != -1) {
        switch (ch) {
        case 'b':
            backend = optarg;
            break;
        case 'v':
            ++verbosity;
            break;
        default:
            std::cout << "Usage: " << name << " [-v] [-b backend]"
                      << "\n";
            exit(1);
        }
    }
    argc -= optind;
    argv += optind;
    if (argc != 0) {
        std::cout << "Usage: " << name << " [-v] [-b backend]"
                  << "\n";
        exit(1);
    }

    mk::ooni::HttpInvalidRequestLineTest()
        .set_backend(backend)
        .set_verbosity(verbosity)
        .on_log([](uint32_t, const char *s) { std::cout << s << "\n"; })
        .run();
}
