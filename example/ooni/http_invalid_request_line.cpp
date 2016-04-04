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
    bool verbose = false;
    std::string name = argv[0];
    char ch;

    while ((ch = getopt(argc, argv, "b:v")) != -1) {
        switch (ch) {
        case 'b':
            backend = optarg;
            break;
        case 'v':
            verbose = true;
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
        .set_verbose(verbose)
        .on_log([](int, const char *s) { std::cout << s << "\n"; })
        .run();
}
