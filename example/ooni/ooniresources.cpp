// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include <measurement_kit/ooni.hpp>
#include <string>
#include <unistd.h>

#define USAGE "ooniresources [-v] [-d dir]"

using namespace mk::ooni;
using namespace mk;

int main(int argc, char **argv) {

    Settings settings;
    for (int ch; (ch = getopt(argc, argv, "d:v")) != -1; ) {
        switch (ch) {
        case 'd':
            settings["ooni/resources_destdir"] = std::string{optarg};
            break;
        case 'v':
            increase_verbosity();
            break;
        default:
            fprintf(stderr, "%s\n", USAGE);
            exit(1);
            // NOTREACHED
        }
    }
    argc -= optind, argv += optind;

    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        mk::ooni::resources::get_latest_release(
            [=](Error error, std::string latest) {
                if (error) {
                    fprintf(stderr, "error: %s\n", error.what());
                    reactor->stop();
                    return;
                }
                mk::ooni::resources::get_resources(
                    latest, "ALL",
                    [=](Error error) {
                        if (error) {
                            fprintf(stderr, "error: %s\n", error.what());
                            /* FALLTHROUGH */
                        }
                        reactor->stop();
                    }, settings);
            }, settings);
    });

    return 0;
}
