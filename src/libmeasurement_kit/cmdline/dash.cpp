// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/libmeasurement_kit/common/utils.hpp"
#include <functional>
#include <iostream>
#include <measurement_kit/cmdline.hpp>
#include <measurement_kit/ext.hpp>
#include <measurement_kit/http.hpp>
#include <measurement_kit/neubot.hpp>
#include <stdlib.h>
#include <string>
#include <unistd.h>

namespace mk {
namespace cmdline {
namespace dash {

static const char *kv_usage = "usage: %s dash [-vn] [-a address]\n";

int main(const char *progname, int argc, char **argv) {
    Settings settings;

    for (int ch; (ch = getopt(argc, argv, "a:vn")) != -1; ) {
        switch (ch) {
        case 'a':
            settings["url"] = optarg;
            break;
        case 'n':
            settings["negotiate"] = "false";
            break;
        case 'v':
            increase_verbosity();
            break;
        default:
            fprintf(stderr, kv_usage, progname);
            return 1;
            // NOTREACHED
        }
    }
    argc -= optind, argv += optind;
    if (argc > 0) {
        fprintf(stderr, kv_usage, progname);
        return 1;
    }

    loop_with_initial_event([=]() {
        neubot::negotiate::run([=](Error error) {
            debug("Error: %d", error.code);
            break_loop();
        }, settings);
    });

    return 0;
}

} // namespace dash
} // namespace cmdline
} // namespace mk
