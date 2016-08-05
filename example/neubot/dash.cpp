// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/common/utils.hpp"
#include <functional>
#include <iostream>
#include <measurement_kit/common.hpp>
#include <measurement_kit/ext.hpp>
#include <measurement_kit/http.hpp>
#include <measurement_kit/neubot.hpp>
#include <stdlib.h>
#include <string>
#include <unistd.h>

using namespace mk;
using namespace mk::neubot::negotiate;
using namespace mk::net;
using namespace mk::http;

static const char *kv_usage =
    "usage: ./example/http/dash [-vn] [-a address]\n";

int main(int argc, char **argv) {
    Settings settings;
    char ch;

    while ((ch = getopt(argc, argv, "a:vn")) != -1) {
        switch (ch) {
        case 'a':
            settings["url"] = optarg;
            break;
        case 'n':
            settings["negotiate"] = "false";
            break;
        case 'v':
            increase_verbosity();
            break;
        default:
            std::cout << kv_usage;
            exit(1);
        }
    }

    loop_with_initial_event([=]() {
        run([=](Error error) {
            debug("Error: %d", (int) error);

            break_loop();
        }, settings);
    });
}
