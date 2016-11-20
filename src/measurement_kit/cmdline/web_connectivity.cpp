// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../cmdline/cmdline.hpp"
#include <measurement_kit/nettests.hpp>

#include <iostream>

namespace mk {
namespace cmdline {
namespace web_connectivity {

#define USAGE "usage: %s [-nv] [-b backend] [-N nameserver] file_name\n"

int main(const char *, int argc, char **argv) {
    std::string backend = "https://a.collector.test.ooni.io:4444";
    std::string nameserver = "8.8.8.8";
    std::string name = argv[0];
    uint32_t verbosity = 0;
    mk::nettests::WebConnectivityTest test;
    int ch;

    while ((ch = mkp_getopt(argc, argv, "b:N:nv")) != -1) {
        switch (ch) {
        case 'b':
            backend = mkp_optarg;
            break;
        case 'N':
            nameserver = mkp_optarg;
            break;
        case 'n':
            test.set_options("no_collector", true);
            break;
        case 'v':
            ++verbosity;
            break;
        default:
            fprintf(stderr, USAGE, name.c_str());
            exit(1);
        }
    }
    argc -= mkp_optind;
    argv += mkp_optind;
    if (argc != 1) {
        fprintf(stderr, USAGE, name.c_str());
        exit(1);
    }

    test
        .set_options("backend", backend)
        .set_options("nameserver", nameserver)
        .set_options("geoip_country_path", "GeoIP.dat")
        .set_options("geoip_asn_path", "GeoIPASNum.dat")
        .set_verbosity(verbosity)
        .set_input_filepath(argv[0])
        .on_log([](uint32_t, const char *s) { std::cout << s << "\n"; })
        .run();

    return 0;
}

} // namespace web_connectivity
} // namespace cmdline
} // namespace mk
