// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/ooni.hpp>
#include <iostream>
#include <string>
#include <unistd.h>

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

int main(int argc, char **argv) {
    std::string url = collector::default_collector_url();
    int ch;

    while ((ch = getopt(argc, argv, "c:v")) != -1) {
        switch (ch) {
        case 'c':
            url = optarg;
            break;
        case 'v':
            increase_verbosity();
            break;
        default:
            std::cout << USAGE << "\n";
            exit(1);
        }
    }
    argc -= optind;
    argv += optind;

    loop_with_initial_event([&]() {
        upload_report(url, 0, argv);
    });
}
