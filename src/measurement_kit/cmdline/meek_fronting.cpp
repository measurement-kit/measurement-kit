// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../cmdline/cmdline.hpp"
#include <measurement_kit/nettests.hpp>

#include <iostream>

namespace mk {
namespace cmdline {
namespace meek_fronting {

int main(const char *, int argc, char **argv) {
//    std::string backend = "http://213.138.109.232/";
//    uint32_t verbosity = 0;
//    std::string name = argv[0];
//    mk::nettests::HttpInvalidRequestLineTest test;
//    int ch;
//
//    while ((ch = mkp_getopt(argc, argv, "b:nv")) != -1) {
//        switch (ch) {
//        case 'b':
//            backend = mkp_optarg;
//            break;
//        case 'n':
//            test.set_options("no_collector", true);
//            break;
//        case 'v':
//            ++verbosity;
//            break;
//        default:
//            std::cout << "Usage: " << name << " [-nv] [-b backend]"
//                      << "\n";
//            exit(1);
//        }
//    }
//    argc -= mkp_optind;
//    argv += mkp_optind;
//    if (argc != 0) {
//        std::cout << "Usage: " << name << " [-nv] [-b backend]"
//                  << "\n";
//        exit(1);
//    }
//
//    test
//        .set_options("backend", backend)
//        .set_options("geoip_country_path", "GeoIP.dat")
//        .set_options("geoip_asn_path", "GeoIPASNum.dat")
//        .set_verbosity(verbosity)
//        .on_log([](uint32_t, const char *s) { std::cout << s << "\n"; })
//        .run();
//
    std::cout << "not implemented" << std::endl;
    return 0;
}

} // namespace meek_fronting
} // namespace cmdline
} // namespace mk
