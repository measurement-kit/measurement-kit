// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../cmdline.hpp"
#include <measurement_kit/ext/json.hpp>
#include <measurement_kit/ndt.hpp>

namespace ndt {

#define USAGE                                                                  \
    "usage: measurement_kit [options] ndt [-m metro] [-N tool] [-p port]\n"    \
    "                       [-T phase] [host]\n"                               \
    "\n"                                                                       \
    "Available tool names for mlab-ns: ndt, neubot (default: ndt)\n"           \
    "Available phases: download, download-ext, none, upload\n"                 \
    "  (default: -T download -T upload)\n"

int main(std::list<Callback<BaseTest &>> &initializers, int argc, char **argv) {
    NdtTest test;
    int test_suite = 0;
    int use_default_test_suite = true;

    for (int ch; (ch = getopt(argc, argv, "m:N:p:T:")) != -1;) {
        switch (ch) {
        case 'm':
            test.set_options("mlabns/policy", "metro");
            test.set_options("mlabns/metro", optarg);
            break;
        case 'N':
            test.set_options("mlabns_tool_name", optarg);
            break;
        case 'p':
            test.set_options("port", optarg);
            break;
        case 'T':
            use_default_test_suite = false;
            if (strcmp(optarg, "download") == 0) {
                test_suite |= MK_NDT_DOWNLOAD;
            } else if (strcmp(optarg, "download-ext") == 0) {
                test_suite |= MK_NDT_DOWNLOAD_EXT;
            } else if (strcmp(optarg, "none") == 0) {
                test_suite = 0;
            } else if (strcmp(optarg, "upload") == 0) {
                test_suite |= MK_NDT_UPLOAD;
            } else {
                fprintf(stderr, "invalid parameter for -T option: %s", optarg);
                exit(1);
            }
            break;
        default:
            fprintf(stderr, "%s\n", USAGE);
            exit(1);
            /* NOTREACHED */
        }
    }
    argc -= optind, argv += optind;
    if (argc > 1) {
        fprintf(stderr, "%s\n", USAGE);
        exit(1);
        /* NOTREACHED */
    }

    // If the user expressed preference force test suite, otherwise the
    // code would use the default test suite (download|upload).
    if (!use_default_test_suite) {
        test.set_options("test_suite", test_suite);
    }

    if (argc == 1) {
        test.set_options("address", argv[0]);
    }

    ndt_init(initializers, test)
          .on_entry([](std::string s) {
              nlohmann::json doc = nlohmann::json::parse(s);
              auto simple = doc["test_keys"]["simple"];
              auto advanced = doc["test_keys"]["advanced"];
              printf("\nTest summary\n");
              printf("------------\n");
              double download = simple["download"];
              double upload = simple["upload"];
              printf("Upload speed: %.2f kbit/s\n", upload);
              printf("Download speed: %.2f kbit/s\n", download);
              double packet_loss = advanced["packet_loss"];
              double out_of_order = advanced["out_of_order"];
              double max_rtt = advanced["max_rtt"];
              double avg_rtt = advanced["avg_rtt"];
              double min_rtt = advanced["min_rtt"];
              long mss = advanced["mss"];
              long timeouts = advanced["timeouts"];
              printf("Packet loss rate: %.2f%%\n", packet_loss * 100.0);
              printf("Out of order: %.2f%%\n", out_of_order * 100.0);
              printf("RTT (min/avg/max): %.2f/%.2f/%.2f ms\n", min_rtt, avg_rtt,
                     max_rtt);
              printf("MSS: %ld\n", mss);
              printf("Timeouts: %ld\n", timeouts);
              printf("\n");
          })
          .run();
    return 0;
}

} // namespace ndt
