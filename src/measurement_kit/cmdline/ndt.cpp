// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../cmdline/cmdline.hpp"
#include <measurement_kit/ext/json.hpp>
#include <measurement_kit/nettests.hpp>
#include <measurement_kit/ndt.hpp>

#include <string.h>
#include <stdio.h>

#include <iostream>

namespace mk {
namespace cmdline {
namespace ndt {

using namespace mk::nettests;

static const char *kv_usage =
  "usage: measurement_kit ndt [-nv] [-C /path/to/ca.bundle] [-p port]\n"
  "                           [-N tool_name] [-T phase] [host]\n"
  "\n"
  "Available tool names for mlab-ns: ndt, neubot (default: ndt)\n"
  "Available phases: download, download-ext, none, upload\n"
  "  (default: -T download -T upload)\n";

int main(const char *, int argc, char **argv) {

    NdtTest test;
    int ch;
    int test_suite = 0;
    int use_default_test_suite = true;
    while ((ch = mkp_getopt(argc, argv, "C:nN:p:T:v")) != -1) {
        switch (ch) {
        case 'C':
            test.set_options("net/ca_bundle_path", mkp_optarg);
            break;
        case 'N':
            test.set_options("mlabns_tool_name", mkp_optarg);
            break;
        case 'n':
            test.set_options("no_collector", true);
            break;
        case 'p':
            test.set_options("port", mkp_optarg);
            break;
        case 'T':
            use_default_test_suite = false;
            if (strcmp(mkp_optarg, "download") == 0) {
                test_suite |= MK_NDT_DOWNLOAD;
            } else if (strcmp(mkp_optarg, "download-ext") == 0) {
                test_suite |= MK_NDT_DOWNLOAD_EXT;
            } else if (strcmp(mkp_optarg, "none") == 0) {
                test_suite = 0;
            } else if (strcmp(mkp_optarg, "upload") == 0) {
                test_suite |= MK_NDT_UPLOAD;
            } else {
                warn("invalid parameter for -T option: %s", mkp_optarg);
                exit(1);
            }
            break;
        case 'v':
            test.increase_verbosity();
            break;
        default:
            std::cout << kv_usage;
            exit(1);
        }
    }
    argc -= mkp_optind;
    argv += mkp_optind;

    // If the user expressed preference force test suite, otherwise the
    // code would use the default test suite (download|upload).
    if (!use_default_test_suite) {
        test.set_options("test_suite", test_suite);
    }

    if (argc > 1) {
        std::cout << kv_usage;
        exit(1);
    } else if (argc == 1) {
        test.set_options("address", argv[0]);
    }

    test
        .on_progress([](double prog, const char *s) {
            printf("Progress: %.2f%%: %s\n", prog * 100.0, s);
        })
        .set_options("geoip_country_path", "GeoIP.dat")
        .set_options("geoip_asn_path", "GeoIPASNum.dat")
        .run();

    return 0;
}

} // namespace ndt
} // namespace cmdline
} // namespace mk
