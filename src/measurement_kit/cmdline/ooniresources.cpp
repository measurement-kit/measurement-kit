// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/ooni.hpp>
#include <string>
#include <unistd.h>

#include "../cmdline/cmdline.hpp"

namespace mk {
namespace cmdline {
namespace ooniresources {

#define USAGE "ooniresources [-v]" 

using namespace mk::ooni;
using namespace mk;

int main(const char *, int argc, char **argv) {

    for (int ch; (ch = getopt(argc, argv, "v")) != -1; ) {
        switch (ch) {
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

    loop_with_initial_event([=]() {
        mk::ooni::resources::get_latest_release(
            [=](Error error, std::string latest) {
                if (error) {
                    fprintf(stderr, "error: %s\n", error.explain().c_str());
                    break_loop();
                    return;
                }
                mk::ooni::resources::get_resources(
                    latest, "ALL",
                    [=](Error error) {
                        if (error) {
                            fprintf(stderr, "error: %s\n",
                                    error.explain().c_str());
                            /* FALLTHROUGH */
                        }
                        break_loop();
                    });
            });
    });

    return 0;
}

} // namespace ooniresources
} // namespace cmdline
} // namespace mk
