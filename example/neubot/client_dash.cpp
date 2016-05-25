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

#define DASH_MAX_NEGOTIATION 512

using namespace mk;
using namespace mk::neubot;
using namespace mk::net;
using namespace mk::http;
using json = nlohmann::json;

void loop_req_negotiate(int, Var<Transport>);
void run_negotiation(Settings, int);

static const char *kv_usage =
    "usage: ./example/http/client_dash [-vn] [-a address]\n";

int main(int argc, char **argv) {
    Settings settings;
    settings["http/url"] = "http://127.0.0.1";
    char ch;
    int negotiate = 1;

    while ((ch = getopt(argc, argv, "a:vn")) != -1) {
        switch (ch) {
        case 'a':
            settings["http/url"] = optarg;
            break;
        case 'n':
            negotiate = 0;
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
        run_negotiation(settings, negotiate);
    });
}

void loop_req_negotiate(int iteraction, Var<Transport> transport) {

    Settings settings;
    std::array<int, 20> DASH_RATES{{100,  150,  200,  250,  300,   400,  500,
                                    700,  900,  1200, 1500, 2000,  2500, 3000,
                                    4000, 5000, 6000, 7000, 10000, 20000}};
    std::string url = "http://127.0.0.1/negotiate/dash";
    json::object_t value = {{"dash_rates", DASH_RATES}};
    json json_body(value);
    std::string body = json_body.dump();

    settings["http/url"] = url;
    settings["http/method"] = "POST";

    if (iteraction > DASH_MAX_NEGOTIATION) {
        transport->close([=]() { break_loop(); });
        return;
    };

    request_sendrecv(
        transport, settings,
        {
            {"Content-Type", "application/json"}, {"Authorization", ""},
        },
        body,
        [=](Error error, Var<Response> res) {
            if (error || (res->status_code != 200)) {
                std::cout << "Error: " << (int)error << "\n";
                break_loop();
                return;
            }

            auto respbody = json::parse(res->body);
            std::string auth = respbody.at("authorization");
            auto queue_pos = respbody.at("queue_pos");
            auto real_address = respbody.at("real_address");
            auto unchoked = respbody.at("unchoked");

            if (auth == "") {
                loop_req_negotiate(iteraction + 1, transport);
            } else {
                run(settings, [=] (Error) {
                    break_loop();
                }, auth);
            }

        });
}

void run_negotiation(Settings settings, int negotiation) {

    if (negotiation == 0) {
        run(settings, [=](Error) { break_loop(); });
        return;
    }

    request_connect(settings, [=](Error error, Var<Transport> transport) {
        if (error) {
            std::cout << "Error: " << (int)error;
            break_loop();
        }

        loop_req_negotiate(1, transport);

        return;
    });
}
