// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include <measurement_kit/ooni.hpp>

#include <iostream>

#include "src/measurement_kit/portable/getopt.h"

#define USAGE "oonireport [-v] [-c collector-base-url] filepath [...]" 

using namespace mk::ooni;
using namespace mk;

static void upload_report(std::string url, int index, char **argv, SharedPtr<Reactor> reactor) {
    if (argv[index] == nullptr) {
        reactor->stop();
        return;
    }
    info("submitting report %s...", argv[index]);
    collector::submit_report(argv[index], url, [=](Error err) {
        info("submitting report %s... %d", argv[index], err.code);
        reactor->call_soon([=]() {
            debug("scheduling submit of next report...");
            upload_report(url, index + 1, argv, reactor);
        });
    });
}

int main(int argc, char **argv) {
    std::string url = collector::production_collector_url();
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

    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        upload_report(url, 0, argv, reactor);
    });

    return 0;
}
