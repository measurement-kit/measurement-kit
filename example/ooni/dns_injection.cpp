// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/ooni.hpp>
#include <iostream>
#include <string>
#include <unistd.h>

int main(int argc, char **argv) {
    std::string backend = "8.8.8.1";
    std::string name = argv[0];
    uint32_t verbosity = 0;
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

    mk::ooni::DnsInjection()
        .set_options("backend", backend)
        .set_options("geoip_country_path", "test/fixtures/GeoIP.dat")
        .set_options("geoip_asn_path", "test/fixtures/GeoIPASNum.dat")
        .set_verbosity(verbosity)
        .set_input_filepath(argv[0])
        .on_log([](uint32_t, const char *s) { std::cout << s << "\n"; })
        .run();
}
