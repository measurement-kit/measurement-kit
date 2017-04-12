// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_NEUBOT_DASH_IMPL_HPP
#define SRC_LIBMEASUREMENT_KIT_NEUBOT_DASH_IMPL_HPP

#include "../common/utils.hpp"
#include <measurement_kit/http.hpp>
#include <measurement_kit/mlabns.hpp>
#include <measurement_kit/neubot.hpp>

#define DASH_MAX_ITERATION 15
#define DASH_SECONDS 2
#define DASH_MAX_NEGOTIATION 512

namespace mk {
namespace neubot {
namespace dash {

std::vector<int> dash_rates(); // Implemented in dash.cpp

static inline size_t select_lower_rate_index(int speed_kbit) {
    size_t rate_index = 0;
    while (dash_rates()[rate_index] < speed_kbit &&
           rate_index < dash_rates().size()) {
        rate_index++;
    }
    if (rate_index > 0) { // Make sure we don't underflow
        rate_index -= 1;
    }
    return rate_index;
}

template <MK_MOCK_AS(http::request_send, http_request_send),
          MK_MOCK_AS(http::request_recv_response, http_request_recv_response)>
void run_loop_(Var<Entry> measurements, Var<Transport> txp,
               int speed_kbit, std::string auth_token, Settings settings,
               Var<Reactor> reactor, Var<Logger> logger, Callback<Error> cb,
               int iteration = 1) {
    if (iteration > DASH_MAX_ITERATION) {
        logger->debug("dash: completed all iterations");
        cb(NoError());
        return;
    }

    size_t rate_index = select_lower_rate_index(speed_kbit);
    int rate_kbit = dash_rates()[rate_index];
    int count = ((rate_kbit * 1000) / 8) * DASH_SECONDS;
    std::string path = "/dash/download/";
    path += std::to_string(count);
    settings["http/path"] = path;
    double saved_times = mk::time_now();
    logger->debug("dash: requesting '%s'", path.c_str());

    http_request_send(
        txp, settings,
        {
            {"Authorization", auth_token},
        },
        "", [=](Error error, Var<Request> req) {
            if (error) {
                logger->warn("dash: request failed: %s",
                             error.explain().c_str());
                cb(error);
                return;
            }
            http_request_recv_response(
                txp,
                [=](Error error, Var<Response> res) {
                    if (error) {
                        logger->warn("dash: cannot receive response: %s",
                                     error.explain().c_str());
                        cb(error);
                        return;
                    }
                    if (res || res->status_code != 200) {
                        logger->warn("dash: invalid response code: %s",
                                     error.explain().c_str());
                        cb(HttpRequestFailedError());
                        return;
                    }
                    res->request = req;
                    double new_times = mk::time_now();
                    double length = res->body.length();
                    double time_elapsed = new_times - saved_times;
                    if (time_elapsed <= 0) { // For robustness
                        logger->warn("dash: invalid time error");
                        cb(InvalidTimeError());
                        return;
                    }
                    // TODO: we should fill all the required fields
                    Entry entry = {//{"connect_time", self.rtts[0]}
                                   //{"delta_user_time", delta_user_time}
                                   //{"delta_sys_time", delta_sys_time}
                                   {"elapsed", time_elapsed},
                                   {"elapsed_target", DASH_SECONDS},
                                   //{"internal_address", stream.myname[0]}
                                   {"iteration", iteration},
                                   //{"platform", sys.platform}
                                   {"rate", rate_kbit},
                                   //{"real_address", self.parent.real_address}
                                   //{"received", received}
                                   //{"remote_address", stream.peername[0]}
                                   //{"request_ticks", self.saved_ticks}
                                   //{"timestamp", mk::time_now}
                                   //{"uuid", self.conf.get("uuid")}
                                   {"version", MEASUREMENT_KIT_VERSION}};
                    measurements->push_back(entry);
                    double speed = length / time_elapsed;
                    double s_k = (speed * 8) / 1000;
                    logger->debug("[%d/%d] rate: %d kbit/s, speed: %.2f "
                                  "kbit/s, elapsed: %.2f s",
                                  iteration, DASH_MAX_ITERATION, rate_kbit, s_k,
                                  time_elapsed);
                    if (time_elapsed > DASH_SECONDS) {
                        // If the rate is too high, scale it down
                        double relerr = 1 - (time_elapsed / DASH_SECONDS);
                        s_k *= relerr;
                        if (s_k < 0) {
                            s_k = dash_rates()[0];
                        }
                    }
                    reactor->call_soon([=]() {
                        run_loop_(measurements, txp, s_k, auth_token,
                                  settings, reactor, logger, cb, iteration + 1);
                    });
                },
                reactor, logger);
        });
}

template <MK_MOCK_AS(http::request_connect, http_request_connect)>
void run_with_impl(std::string measurement_server_url, std::string auth_token,
                   Settings settings, Var<Reactor> reactor, Var<Logger> logger,
                   Callback<Error, Var<Entry>> cb) {
    Var<Entry> measurements{new Entry};
    settings["http/url"] = measurement_server_url;
    logging->debug("start dash test with: %s", measurement_server_url.c_str());
    http_request_connect(
        settings,
        [=](Error error, Var<Transport> txp) {
            if (error) {
                logger->warn("dash: cannot connect to server: %s",
                             error.explain().c_str());
                cb(error, measurements);
                return;
            }
            run_loop_(measurements, txp, dash_rates()[0], auth_token,
                      settings, reactor, logger, [=](Error error) {
                          txp->close([=]() {
                              logger->debug("dash test complete");
                              cb(error, measurements);
                          });
                      });
        },
        reactor, logger);
}

template <MK_MOCK_AS(http::request_sendrecv, http_request_sendrecv)>
void collect(Var<Entry> measurements, Var<Transport> txp, std::string auth,
             Settings settings, Var<Reactor> reactor, Var<Logger> logger,
             Callback<Error> cb) {
    std::string body = measurements->dump();
    settings["http/method"] = "POST";
    settings["http/path"] = "/collect/dash";
    http_request_sendrecv(
        txp, settings,
        {
            {"Content-Type", "application/json"}, {"Authorization", auth},
        },
        body,
        [=](Error error, Var<Response> res) {
            if (error) {
                logger->warn("neubot: collect failed: %s",
                             error.explain().c_str());
            } else if (!res || res->status_code != 200) {
                error = HttpRequestFailedError();
            }
            // XXX: we probably don't own the transport here
            txp->close([=]() { cb(error); });
        },
        reactor, logger);
}

template <MK_MOCK_AS(http::request_sendrecv, http_request_sendrecv)>
void negotiate_loop_(Var<report::Entry> measurements, Var<net::Transport> txp,
                     Settings settings, Var<Reactor> reactor,
                     Var<Logger> logger, Callback<Error> callback,
                     int iteration = 0, std::string auth_token = 0) {
    Entry value = {{"dash_rates", dash_rates()}};
    std::string body = value.dump();
    settings["http/path"] = "/negotiate/dash";
    settings["http/method"] = "POST";
    if (iteration > DASH_MAX_NEGOTIATION) {
        logger->warn("neubot: too many negotiations");
        cb(TooManyNegotiationsError());
        return;
    }
    http_request_sendrecv(
        txp, settings,
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
            if (!res || res->status_code != 200) {
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
                    negotiate_loop_(measurements, txp, settings, reactor,
                                    logger, cb, iteration + 1, auth);
                });
                return;
            }
            run_with(auth, settings, reactor, logger,
                     [=](Error err, Var<Entry> measurements) {
                         if (err) {
                             logger->warn("neubot: test failed: %s",
                                          error.explain().c_str());
                             cb(err);
                             return;
                         }
                         // FIXME: why the fuck do we call collect() here?
                         collect(auth, measurements, txp, settings, reactor,
                                 logger, cb);
                     });
        },
        reactor, logger);
}

template <MK_MOCK(http::request_connect, http_request_connect)>
void negotiate_with_impl(std::string url, Var<report::Entry> measurements,
                         Settings settings, Var<Reactor> reactor,
                         Var<Logger> logger, Callback<Error> cb) {
    ErrorOr<bool> maybe_negotiate = settings["negotiate"].as_noexcept<bool>();
    if (!maybe_negotiate) {
        logger->warn("neubot: invalid 'negotiate' setting");
        callback(maybe_negotiate.as_error());
        return;
    }
    if (*maybe_negotiate == false) {
        // XXX Here we should be able to run _any_ test
        run_impl(url, "" /* no auth at the beginning */, measurements, settings,
                 reactor, logger, cb);
        return;
    }
    settings["http/url"] = url;
    http_request_connect(
        settings,
        [=](Error error, Var<Transport> txp) {
            if (error) {
                // Note: in this case we don't need to close the transport
                logger->warn("neubot: cannot connect to "
                             "negotiate server: %s",
                             error.explain().c_str());
                cb(error);
                return;
            }
            negotiate_loop_(
                txp, measurements, settings, reactor, logger,
                [=](Error error, std::string auth_token) {
                    if (error) {
                        logger->warn("neubot: negotiate failed: %s",
                                     error.explain().c_str());
                        txp->close([=]() { cb(error); });
                        return;
                    }
                    // TODO: generalize code, allow to run more tests
                    run_impl(url /* XXX: really the same URL?! */,
                             auth_token,
                             measurements, settings, reactor, logger,
                             [=](Error error) {
                                 if (error) {
                                     logger->warn("neubot: test failed: %s",
                                                  error.explain().c_str());
                                     txp->close([=]() { cb(error); });
                                     return;
                                 }
                                 collect_impl(
                                     txp, measurements, settings, reactor,
                                     logger, [=](Error error) {
                                         txp->close([=]() { cb(error); });
                                     });
                             });
                });
        },
        reactor, logger);
}

template <MK_MOCK_AS(mlabns::query, mlabns_query)>
void negotiate_impl(Var<report::Entry> measurements, Settings settings,
                    Var<Reactor> reactor, Var<Logger> logger,
                    Callback<Error> cb) {
    if (settings.find("url") != settings.end()) {
        negotiate_with_impl(settings["url"], measurements, settings, reactor,
                            logger, cb);
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
                     negotiate_with_impl(reply.url, measurements, settings,
                                         reactor, logger, cb);
                 },
                 settings, reactor, logger);
}

} // namespace dash
} // namespace neubot
} // namespace mk
#endif
