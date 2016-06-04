// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/ooni.hpp>
#include <iostream>
#include <string>
#include <unistd.h>

#define USAGE "oonireport [-v] [-c collector] filepath [...]" 

using namespace mk::ooni;
using namespace mk;

static void upload_report(std::string collector, int argc, char **argv) {
    if (collector == "") {
        warn("Default collector not yet supported");
        break_loop();
        return;
    }
    if (argv[argc] == nullptr) {
        break_loop();
        return;
    }
    info("submitting report %s...", argv[argc]);
    submit_report(argv[argc], collector, [=](Error err) {
        info("submitting report %s... %d", argv[argc], err.code);
        call_soon([=]() {
            debug("scheduling submit of next report...");
            upload_report(collector, argc + 1, argv);
        });
    });
}

int main(int argc, char **argv) {
    std::string collector;

    while ((ch = getopt(argc, argv, "c:v")) != -1) {
        switch (ch) {
        case 'c':
            collector = optarg;
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
    if (argc < 1) {
        std::cout << USAGE << "\n";
        exit(1);
    }

    loop_with_initial_event([&]() {
        upload_report(collector, argc, argv);
    });
}
