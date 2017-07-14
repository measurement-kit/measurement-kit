// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../cmdline.hpp"
#include <measurement_kit/ext/json.hpp>

namespace dash {

#define USAGE                                                                  \
    "usage: measurement_kit [options] dash [-b constant_bitrate] [-U uuid] "   \
    "[hostname]"

int main(std::list<Callback<BaseTest &>> &initializers, int argc, char **argv) {
    nettests::DashTest test;
    for (int ch; (ch = getopt(argc, argv, "b:U:")) != -1;) {
        switch (ch) {
        case 'b':
            test.set_options("constant_bitrate", optarg);
            break;
        case 'U':
            test.set_options("uuid", optarg);
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
    if (argc == 1) {
        test.set_options("hostname", argv[0]);
    }

    common_init(initializers, test)
          .on_entry([](std::string s) {
              nlohmann::json doc = nlohmann::json::parse(s);
              auto simple = doc["test_keys"]["simple"];
              printf("\nTest summary\n");
              printf("------------\n");
              double median_bitrate = simple["median_bitrate"];
              double min_playout_delay = simple["min_playout_delay"];
              double connect_latency = simple["connect_latency"];
              printf("Connect latency: %.2f ms\n", 1000.0 * connect_latency);
              printf("Median bitrate: %.2f kbit/s\n", median_bitrate);
              printf("Min. playout delay: %.3f s\n", min_playout_delay);
              printf("\n");
          })
          .run();
    return 0;
}

} // namespace dash
