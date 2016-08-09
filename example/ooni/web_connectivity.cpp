// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/ooni.hpp>
#include <iostream>
#include <string>
#include <unistd.h>

#define USAGE "usage: %s [-v] [-b backend] [-n nameserver] file_name\n"

int main(int argc, char **argv) {
    std::string backend = "https://a.collector.test.ooni.io:4444";
    std::string nameserver = "8.8.8.8";
    std::string name = argv[0];
    uint32_t verbosity = 0;
    char ch;

    while ((ch = getopt(argc, argv, "b:n:v")) != -1) {
        switch (ch) {
        case 'b':
            backend = optarg;
            break;
        case 'n':
            nameserver = optarg;
            break;
        case 'v':
            ++verbosity;
            break;
        default:
            fprintf(stderr, USAGE, name.c_str());
            exit(1);
        }
    }
    argc -= optind;
    argv += optind;
    if (argc != 1) {
        fprintf(stderr, USAGE, name.c_str());
        exit(1);
    }

    mk::ooni::WebConnectivity()
        .set_options("backend", backend)
        .set_options("nameserver", nameserver)
        .set_options("geoip_country_path", "test/fixtures/GeoIP.dat")
        .set_options("geoip_asn_path", "test/fixtures/GeoIPASNum.dat")
        .set_verbosity(verbosity)
        .set_input_filepath(argv[0])
        .on_log([](uint32_t, const char *s) { std::cout << s << "\n"; })
        .run();
}
