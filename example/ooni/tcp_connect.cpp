// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/ooni.hpp>
#include <iostream>
#include <string>
#include <unistd.h>

int main(int argc, char **argv) {
    std::string port = "80";
    std::string name = argv[0];
    uint32_t verbosity = 0;
    char ch;
    while ((ch = getopt(argc, argv, "p:v")) != -1) {
        switch (ch) {
        case 'p':
            port = std::string(optarg);
            break;
        case 'v':
            ++verbosity;
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
    mk::ooni::TcpConnect()
        .set_options("port", port)
        .set_options("geoip_country_path", "test/fixtures/GeoIP.dat")
        .set_options("geoip_asn_path", "test/fixtures/GeoIPASNum.dat")
        .set_input_filepath(argv[0])
        .set_verbosity(verbosity)
        .on_log([](uint32_t, const char *s) { std::cout << s << "\n"; })
        .run();
}
