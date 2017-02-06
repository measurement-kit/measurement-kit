// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../cmdline/cmdline.hpp"
#include <measurement_kit/nettests.hpp>

#include <iostream>

namespace mk {
namespace cmdline {
namespace tcp_connect {

int main(const char *, int argc, char **argv) {
    std::string port = "80";
    std::string name = argv[0];
    uint32_t verbosity = 0;
    mk::nettests::TcpConnectTest test;
    int ch;
    while ((ch = mkp_getopt(argc, argv, "np:v")) != -1) {
        switch (ch) {
        case 'p':
            port = std::string(mkp_optarg);
            break;
        case 'n':
            test.set_options("no_collector", true);
            break;
        case 'v':
            ++verbosity;
            break;
        default:
            std::cout << "Usage: " << name << " [-nv] [-p port] file_name"
                      << "\n";
            exit(1);
        }
    }
    argc -= mkp_optind;
    argv += mkp_optind;
    if (argc != 1) {
        std::cout << "Usage: " << name << " [-nv] [-p port] file_name"
                  << "\n";
        exit(1);
    }
    test
        .set_options("port", port)
        .set_options("geoip_country_path", "GeoIP.dat")
        .set_options("geoip_asn_path", "GeoIPASNum.dat")
        .set_input_filepath(argv[0])
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

} // namespace tcp_connect
} // namespace cmdline
} // namespace mk
