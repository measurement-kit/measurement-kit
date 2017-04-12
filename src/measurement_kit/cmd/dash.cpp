// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../cmdline.hpp"

namespace dash {

#define USAGE "usage: measurement_kit [options] dash [hostname]"

int main(std::list<Callback<BaseTest &>> &initializers, int argc, char **argv) {
    Settings settings;
    for (int ch; (ch = getopt(argc, argv, "")) != -1; ) {
        switch (ch) {
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
        settings["url"] = argv[0];
    }
    loop_with_initial_event([=]() {
        neubot::negotiate::run([=](Error error) {
            if (error) {
                mk::warn("dash test failed: %s", error.explain().c_str());
            }
            break_loop();
        }, settings);
    });
    return 0;
}

} // namespace dash
