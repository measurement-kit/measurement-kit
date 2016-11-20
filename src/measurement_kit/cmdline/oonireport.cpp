// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../cmdline/cmdline.hpp"
#include <measurement_kit/ooni.hpp>

#include <iostream>

namespace mk {
namespace cmdline {
namespace oonireport {

#define USAGE "oonireport [-v] [-c collector-base-url] filepath [...]" 

using namespace mk::ooni;
using namespace mk;

static void upload_report(std::string url, int index, char **argv) {
    if (argv[index] == nullptr) {
        break_loop();
        return;
    }
    info("submitting report %s...", argv[index]);
    collector::submit_report(argv[index], url, [=](Error err) {
        info("submitting report %s... %d", argv[index], err.code);
        call_soon([=]() {
            debug("scheduling submit of next report...");
            upload_report(url, index + 1, argv);
        });
    });
}

int main(const char *, int argc, char **argv) {
    std::string url = collector::production_collector_url();
    int ch;

    while ((ch = mkp_getopt(argc, argv, "c:v")) != -1) {
        switch (ch) {
        case 'c':
            url = mkp_optarg;
            break;
        case 'v':
            increase_verbosity();
            break;
        default:
            std::cout << USAGE << "\n";
            exit(1);
        }
    }
    argc -= mkp_optind;
    argv += mkp_optind;

    loop_with_initial_event([&]() {
        upload_report(url, 0, argv);
    });

    return 0;
}

} // namespace oonireport
} // namespace cmdline
} // namespace mk
