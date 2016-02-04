// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

//
// This example will run a synchronous tcp-connection test
//

#include <measurement_kit/ooni.hpp>
#include <iostream>
#include <string>
#include <unistd.h>

int main(int argc, char **argv) {
    std::string port = "80";
    std::string name = argv[0];
    bool verbose = false;
    char ch;
    while ((ch = getopt(argc, argv, "p:v")) != -1) {
        switch (ch) {
        case 'p':
            port = std::string(optarg);
            break;
        case 'v':
            verbose = true;
            break;
        default:
            std::cout << "Usage: " << name << " [-v] [-p port] file_name"
                      << "\n";
            exit(1);
        }
    }
    argc -= optind;
    argv += optind;
    if (argc != 1) {
        std::cout << "Usage: " << name << " [-v] [-p port] file_name"
                  << "\n";
        exit(1);
    }
    mk::ooni::TcpConnectTest()
        .set_port(port)
        .set_input_file_path(argv[0])
        .set_verbose(verbose)
        .on_log([](const char *s) { std::cout << s << "\n"; })
        .run();
}
