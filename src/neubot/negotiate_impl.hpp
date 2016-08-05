// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/common/utils.hpp"
#include <functional>
#include <iostream>
#include <measurement_kit/common.hpp>
#include <measurement_kit/http.hpp>
#include <measurement_kit/mlabns.hpp>
#include <measurement_kit/neubot.hpp>
#include <measurement_kit/report.hpp>
#include <stdlib.h>
#include <string>
#include <unistd.h>

#define DASH_MAX_NEGOTIATION 512

using namespace mk;
using namespace mk::neubot;
using namespace mk::net;
using namespace mk::http;
using namespace mk::report;
using namespace mk::mlabns;

namespace mk {
namespace neubot {
namespace negotiate {

template <MK_MOCK_NAMESPACE(http, request_sendrecv)>
void collect(Var<Transport> transport, Callback<Error> cb, std::string auth,
             Var<Entry> measurements, Settings settings, Var<Reactor> reactor,
             Var<Logger> logger) {
    std::string body = measurements->dump();
    settings["http/method"] = "POST";
    settings["http/path"] = "/collect/dash";

    request_sendrecv(
        transport, settings,
        {
            {"Content-Type", "application/json"}, {"Authorization", auth},
        },
        body,
        [=](Error error, Var<Response> res) {
            if (error || (res->status_code != 200)) {
                logger->warn("Error: %d", (int)error);
                cb(HttpRequestFailedError());
                return;
            }

            transport->close([=]() { cb(NoError()); });

        },
        reactor, logger);
}

template <MK_MOCK_NAMESPACE(http, request_sendrecv)>
void loop_negotiate(Var<Transport> transport, Callback<Error> cb,
                    Settings settings, Var<Reactor> reactor, Var<Logger> logger,
                    int iteration = 0) {

    std::array<int, 20> DASH_RATES{{100,  150,  200,  250,  300,   400,  500,
                                    700,  900,  1200, 1500, 2000,  2500, 3000,
                                    4000, 5000, 6000, 7000, 10000, 20000}};
    Entry value = {{"dash_rates", DASH_RATES}};
    std::string body = value.dump();

    settings["http/path"] = "/negotiate/dash";
    settings["http/method"] = "POST";

    if (iteration > DASH_MAX_NEGOTIATION) {
        std::cout << "Too many negotiations\n";
        transport->close([=]() { cb(TooManyNegotiationsError()); });
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
                std::cout << "Error: " << (int)error;
                cb(HttpRequestFailedError());
                return;
            }

            auto respbody = nlohmann::json::parse(res->body);
            std::string auth = respbody.at("authorization");
            auto queue_pos = respbody.at("queue_pos");
            auto real_address = respbody.at("real_address");
            int unchoked = respbody.at("unchoked");

            // XXX
            if (unchoked == 0) {
                reactor->call_soon([=]() {
                    loop_negotiate(transport, cb, settings, reactor, logger,
                                   iteration + 1);
                });

            } else {
                dash::run(
                    settings,
                    [=](Error err, Var<Entry> measurements) {
                        if (err) {
                            logger->warn("Error: %d", (int)error);
                            cb(err);
                            return;
                        }

                        collect(transport,
                                [=](Error err) {
                                    if (err) {
                                        logger->warn("Error: %d", (int)error);
                                        cb(err);
                                        return;
                                    }

                                    cb(NoError());
                                },
                                auth, measurements, settings, reactor, logger);

                    },
                    auth);
            }

        },
        reactor, logger);
}

template <MK_MOCK_NAMESPACE(mlabns, query)>
void run_impl(Callback<Error> cb, Settings settings, Var<Reactor> reactor,
              Var<Logger> logger) {

    if (settings["url"] != "") {

        settings["http/url"] = settings["url"];

        if (settings["negotiate"] == "false") {
            dash::run(settings, [=](Error error, Var<Entry>) {
                if (error) {
                    logger->warn("Error: %d", (int)error);
                    cb(error);
                    return;
                }
            });
            return;
        }

        request_connect(settings,
                        [=](Error error, Var<Transport> transport) {

                            if (error) {
                                logger->warn("Error: %d", (int)error);
                                transport->close([=]() { cb(error); });
                                return;
                            }

                            loop_negotiate(transport, cb, settings, reactor,
                                           logger);

                            return;
                        },
                        reactor, logger);
        return;
    }

    query("neubot", [=](Error error, mlabns::Reply reply) {
        if (error) {
            logger->warn("Error: %d", (int)error);
            cb(error);
            return;
        }

        Settings s = settings;

        s["http/url"] = reply.url;

        if (s["negotiate"] == "false") {
            dash::run(s, [=](Error error, Var<Entry>) {
                if (error) {
                    logger->warn("Error: %d", (int)error);
                    cb(error);
                    return;
                }
            });

            return;
        }

        request_connect(s,
                        [=](Error error, Var<Transport> transport) {

                            if (error) {
                                logger->warn("Error: %d", (int)error);
                                transport->close([=]() { cb(error); });
                                return;
                            }

                            loop_negotiate(transport, cb, s, reactor, logger);

                            return;
                        },
                        reactor, logger);

    });

    return;
}

} // namespace negotiate
} // namespace neubot
} // namespace mk
