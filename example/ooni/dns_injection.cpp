// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

//
// This example will run a synchronous dns-injection test
//

#include <measurement_kit/ooni.hpp>
#include <iostream>
#include <string>
#include <unistd.h>

int main(int argc, char **argv) {
    std::string backend = "8.8.8.1";
    std::string name = argv[0];
    bool verbose = false;
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
            std::cout << "Usage: " << name << " [-v] [-b backend] file_name"
                      << "\n";
            exit(1);
        }
    }
    argc -= optind;
    argv += optind;
    if (argc != 1) {
        std::cout << "Usage: " << name << " [-v] [-b backend] file_name"
                  << "\n";
        exit(1);
    }

    mk::ooni::DnsInjectionTest()
        .set_backend(backend)
        .set_verbose(verbose)
        .set_input_file_path(argv[0])
        .on_log([](const char *s) { std::cout << s << "\n"; })
        .run();
}
