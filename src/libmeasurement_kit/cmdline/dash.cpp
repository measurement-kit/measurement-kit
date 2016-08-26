// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/cmdline.hpp>
#include <measurement_kit/neubot.hpp>
#include <stdlib.h>
#include <unistd.h>

namespace mk {
namespace cmdline {
namespace dash {

#define USAGE "usage: %s dash [-vn] [hostname]\n"

int main(const char *progname, int argc, char **argv) {
    Settings settings{
        {"mlabns/base_url", "http://mlab-ns.appspot.com/"},
    };
    for (int ch; (ch = getopt(argc, argv, "vn")) != -1; ) {
        switch (ch) {
        case 'n':
            settings["negotiate"] = "false";
            break;
        case 'v':
            increase_verbosity();
            break;
        default:
            fprintf(stderr, USAGE, progname);
            return 1;
            // NOTREACHED
        }
    }
    argc -= optind, argv += optind;
    if (argc > 1) {
        fprintf(stderr, USAGE, progname);
        return 1;
    }
    if (argc == 1) {
        settings["url"] = argv[0];
    }
    loop_with_initial_event([=]() {
        neubot::negotiate::run([=](Error error) {
            if (error) {
                warn("dash test failed: %d", error.code);
            }
            break_loop();
        }, settings);
    });
    return 0;
}

} // namespace dash
} // namespace cmdline
} // namespace mk
