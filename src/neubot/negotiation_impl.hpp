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

namespace mk {
namespace neubot {

static inline void loop_req_negotiate(Var<Transport> transport,
                                        Callback<Error> cb,
                                        Var<Reactor> reactor, Var<Logger> logger,
                                        int iteration = 0) {

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

        if (iteration > DASH_MAX_NEGOTIATION) {
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
                    std::cout << "Error: CIAO" << (int)error;
                    cb(error);
                    return;
                }

                auto respbody = json::parse(res->body);
                std::string auth = respbody.at("authorization");
                auto queue_pos = respbody.at("queue_pos");
                auto real_address = respbody.at("real_address");
                auto unchoked = respbody.at("unchoked");

                //XXX
                if (auth == "") {
                    loop_req_negotiate(transport, cb, reactor, logger, iteration + 1);
                } else {
                    run(settings, [=] (Error err) {
                        if (err) {
                            break_loop();
                            return;
                        }
                    }, auth);
                }

            });
    }

static inline void run_negotiation_impl(Settings settings, Callback<Error> cb,
                            Var<Reactor> reactor, Var<Logger> logger) {

        /*if (settings["http/negotiate"].as<bool>()== true) {
            run(settings, cb);
            return;
        }*/

        request_connect(settings, [=](Error error, Var<Transport> transport) {

            if (error) {
                std::cout << "Error: CIAO\n\n\n\n";
                std::cout << "Error: " << (int)error;
                cb(error);
                return;
            }

            loop_req_negotiate(transport, cb, reactor, logger);

            return;
        },
        reactor, logger);
    }

} // namespace neubot
} // namespace mk
