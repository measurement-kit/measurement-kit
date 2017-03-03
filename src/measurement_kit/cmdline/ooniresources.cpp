// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/nettests.hpp>
#include <string>
#include <unistd.h>

#include "../cmdline/cmdline.hpp"

namespace mk {
namespace cmdline {
namespace ooniresources {

#define USAGE "ooniresources [-v] [-d dir]"

using namespace mk;

int main(const char *, int argc, char **argv) {

    nettests::UpdateResourcesTask task;
    for (int ch; (ch = getopt(argc, argv, "d:v")) != -1; ) {
        switch (ch) {
        case 'd':
            task.set_options("ooni/resources_destdir", optarg);
            break;
        case 'v':
            task.increase_verbosity();
            break;
        default:
            fprintf(stderr, "%s\n", USAGE);
            exit(1);
            // NOTREACHED
        }
    }
    argc -= optind, argv += optind;

    task.on_progress([=](double percent, const char *action) {
        task.runnable->logger->info("%.1f%% %s", 100.0 * percent, action);
    });
    task.run();
    return 0;
}

} // namespace ooniresources
} // namespace cmdline
} // namespace mk
