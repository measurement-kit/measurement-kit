// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../cmdline/cmdline.hpp"
#include <measurement_kit/nettests.hpp>

#include <iostream>

namespace mk {
namespace cmdline {
namespace web_connectivity {

#define USAGE                                                                  \
"usage: %s [-nv] [-b backend] [-N nameserver] [-t runtime] [file_name...]\n"

int main(const char *, int argc, char **argv) {
    std::string backend = "https://b.web-connectivity.th.ooni.io";
    std::string nameserver = "8.8.8.8";
    std::string name = argv[0];
    uint32_t verbosity = 0;
    mk::nettests::WebConnectivityTest test;
    int ch;

    while ((ch = mkp_getopt(argc, argv, "b:N:nt:v")) != -1) {
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
        case 't':
            test.set_options("max_runtime", mkp_optarg);
            break;
        case 'v':
            ++verbosity;
            break;
        default:
            fprintf(stderr, USAGE, name.c_str());
            exit(1);
        }
    }
    argc -= mkp_optind, argv += mkp_optind;
    while (argc > 0) {
        test.add_input_filepath(argv[0]);
        argc -= 1, argv += 1;
    }

    test
        .set_options("backend", backend)
        .set_options("nameserver", nameserver)
        .set_options("geoip_country_path", "GeoIP.dat")
        .set_options("geoip_asn_path", "GeoIPASNum.dat")
        .set_verbosity(verbosity)
        .on_progress([](double progress, std::string msg) {
            std::cout << progress * 100.0 << "%: " << msg << "\n";
        })
        .on_log([](uint32_t, const char *s) {
            std::cerr << s << "\n";
        })
        .run();

    return 0;
}

} // namespace web_connectivity
} // namespace cmdline
} // namespace mk
