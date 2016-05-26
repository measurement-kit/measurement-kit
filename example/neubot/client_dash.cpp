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
using namespace mk::neubot;
using namespace mk::net;
using namespace mk::http;

static const char *kv_usage =
    "usage: ./example/http/client_dash [-vn] [-a address]\n";

int main(int argc, char **argv) {
    Settings settings;
    settings["http/url"] = "http://127.0.0.1";
    settings["negotiate"] = true;
    char ch;

    while ((ch = getopt(argc, argv, "a:vn")) != -1) {
        switch (ch) {
        case 'a':
            settings["http/url"] = optarg;
            break;
        case 'n':
            settings["http/negotiate"] = false;
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
        run_negotiation(settings, [=](Error err) {

            if (err) {
                break_loop();
            }
        });
    });
}
