// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <functional>
#include <iostream>
#include <measurement_kit/common.hpp>
#include <measurement_kit/neubot.hpp>
#include <stdlib.h>
#include <unistd.h>

#define DASH_MAX_ITERATION 15
#define DASH_SECONDS 2

using namespace mk;
using namespace mk::neubot;

static const char *kv_usage =
    "usage: ./example/http/client_dash [-v] [-a address]\n";

int main(int argc, char **argv) {
    Settings settings;
    settings["http/url"] = "http://127.0.0.1";
    char ch;

    while ((ch = getopt(argc, argv, "a:v")) != -1) {
        switch (ch) {
        case 'a':
            settings["http/url"] = optarg;
            break;
        case 'v':
            increase_verbosity();
            break;
        default:
            std::cout << kv_usage;
            exit(1);
        }
    }

    loop_with_initial_event( [=] () {
        run(settings, [=] (Error) {
            break_loop();
        });
    });

}
