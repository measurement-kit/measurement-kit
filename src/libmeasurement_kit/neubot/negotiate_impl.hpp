// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_NEUBOT_NEGOTIATE_IMPL_HPP
#define SRC_LIBMEASUREMENT_KIT_NEUBOT_NEGOTIATE_IMPL_HPP

#include "../common/utils.hpp"
#include "../neubot/utils.hpp"
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

template <MK_MOCK_AS(http::request_sendrecv, http_request_sendrecv)>
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
            if (!!res && res->status_code != 200) {
                error = HttpRequestFailedError();
            }
            if (error) {
                logger->warn("neubot: collect failed: %s",
                             error.explain().c_str());
            }
            transport->close([=]() { cb(error); });
        },
        reactor, logger);
}

/*
 * TODO: part of the negotiation code below is DASH specific. We should
 * refactor code using lambdas so to avoid explicit dependency.
 */

template <MK_MOCK_AS(http::request_sendrecv, http_request_sendrecv)>
void loop_negotiate(Var<Transport> transport, Callback<Error> cb,
                    Settings settings, Var<Reactor> reactor, Var<Logger> logger,
                    int iteration = 0, std::string auth_token = "") {
    Entry value = {{"dash_rates", dash_rates()}};
    std::string body = value.dump();
    settings["http/path"] = "/negotiate/dash";
    settings["http/method"] = "POST";
    if (iteration > DASH_MAX_NEGOTIATION) {
        logger->warn("neubot: too many negotiations");
        cb(TooManyNegotiationsError());
        return;
    }
    request_sendrecv(
        transport, settings,
        {
            {"Content-Type", "application/json"}, {"Authorization", auth_token},
        },
        body,
        [=](Error error, Var<Response> res) {
            if (error) {
                logger->warn("neubot: negotiate failed: %s",
                             error.explain().c_str());
                cb(error);
                return;
            }
            if (!!res && res->status_code != 200) {
                logger->warn("neubot: negotiate failed on server side");
                cb(HttpRequestFailedError());
                return;
            }
            nlohmann::json respbody;
            std::string auth;
            int queue_pos = 0;
            std::string real_address;
            int unchoked = 0;
            try {
                respbody = nlohmann::json::parse(res->body);
                auth = respbody.at("authorization");
                queue_pos = respbody.at("queue_pos");
                real_address = respbody.at("real_address");
                unchoked = respbody.at("unchoked");
            } catch (const std::exception &) {
                logger->warn("neubot: cannot parse negotiate response");
                cb(GenericError());
                return;
            }
            if (!unchoked) {
                reactor->call_soon([=]() {
                    loop_negotiate(transport, cb, settings, reactor, logger,
                                   iteration + 1, auth);
                });
                return;
            }
            dash::run(settings,
                      [=](Error err, Var<Entry> measurements) {
                          if (err) {
                              logger->warn("neubot: test failed: %s",
                                           error.explain().c_str());
                              cb(err);
                              return;
                          }
                          collect(transport, cb, auth, measurements, settings,
                                  reactor, logger);
                      },
                      auth);
        },
        reactor, logger);
}

// TODO: we should probably pass the entry to the caller
template <MK_MOCK_AS(mlabns::query, mlabns_query)>
void run_impl(Callback<Error> cb, Settings settings, Var<Reactor> reactor,
              Var<Logger> logger) {

    auto run_test_with_url = [=](std::string url) {
        Settings settings_copy = settings; // Make a non-const copy
        settings_copy["http/url"] = url;
        if (settings_copy["negotiate"].as<bool>() == false) {
            dash::run(settings_copy, [=](Error error, Var<Entry> /*entry*/) {
                if (error) {
                    logger->warn("neubot: dash failed: %s",
                                 error.explain().c_str());
                    cb(error);
                    return;
                }
            });
            return;
        }
        request_connect(settings_copy,
                        [=](Error error, Var<Transport> transport) {
                            if (error) {
                                logger->warn("neubot: cannot connect to "
                                             "negotiate server: %s",
                                             error.explain().c_str());
                                cb(error);
                                return;
                            }
                            loop_negotiate(transport,
                                           [=](Error error) {
                                               transport->close(
                                                   [=]() { cb(error); });
                                           },
                                           settings_copy, reactor, logger);
                        },
                        reactor, logger);
    };

    if (settings.find("url") != settings.end()) {
        run_test_with_url(settings["url"]);
        return;
    }
    mlabns_query("neubot",
                 [=](Error error, mlabns::Reply reply) {
                     if (error) {
                         logger->warn("neubot: mlabns error: %s",
                                      error.explain().c_str());
                         cb(error);
                         return;
                     }
                     run_test_with_url(reply.url);
                 },
                 settings, reactor, logger);
}

} // namespace negotiate
} // namespace neubot
} // namespace mk
#endif
