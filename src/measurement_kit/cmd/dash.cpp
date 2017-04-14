// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../cmdline.hpp"

namespace dash {

#define USAGE "usage: measurement_kit [options] dash [hostname]"

int main(std::list<Callback<BaseTest &>> &, int argc, char **argv) {
    Settings settings;
    for (int ch; (ch = getopt(argc, argv, "")) != -1;) {
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
    Var<report::Entry> entry{new report::Entry};
    Var<Reactor> reactor = Reactor::global();
    Var<Logger> logger = Logger::global();
    logger->set_verbosity(7);
    logger->warn("here");
    reactor->run_with_initial_event([=]() {
        neubot::dash::negotiate(entry, settings, reactor, logger,
                                [=](Error error) {
                                    if (error) {
                                        logger->warn("dash test failed: %s",
                                                     error.explain().c_str());
                                    }
                                    reactor->stop();
                                });
    });
    return 0;
}

} // namespace dash
