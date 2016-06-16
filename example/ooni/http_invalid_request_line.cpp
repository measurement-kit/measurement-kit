// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

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

    mk::ooni::HttpInvalidRequestLine()
        .set_options("backend", backend)
        .set_options("geoip_country_path", "test/fixtures/GeoIP.dat")
        .set_options("geoip_asn_path", "test/fixtures/GeoIPASNum.dat")
        .set_verbosity(verbosity)
        .on_log([](uint32_t, const char *s) { std::cout << s << "\n"; })
        .run();
}
