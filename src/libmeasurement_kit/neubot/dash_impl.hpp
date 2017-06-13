// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_NEUBOT_DASH_IMPL_HPP
#define SRC_LIBMEASUREMENT_KIT_NEUBOT_DASH_IMPL_HPP

/*
 * Here we implement the measurement methodology described in "Measuring
 * DASH Streaming Performance from the End Users Perspective using Neubot",
 * by S. Basso, A. Servetti, E. Masala, J.C. De Martin.
 *
 * See: https://nexa.polito.it/publications/basso2014measuring
 */

#include <measurement_kit/http.hpp>
#include <measurement_kit/mlabns.hpp>
#include <measurement_kit/neubot.hpp>

#include "../common/utils.hpp"
#include "../ext/sole.hpp"

#define DASH_MAX_ITERATIONS 60
#define DASH_SECONDS 2
#define MAX_NEGOTIATIONS 512

namespace mk {
namespace neubot {
namespace dash {

const std::vector<int> &dash_rates(); // Implemented in dash.cpp

static inline size_t select_lower_rate_index(int speed_kbit) {
    size_t rate_index = 0;
    while (dash_rates()[rate_index] < speed_kbit &&
           rate_index < dash_rates().size()) {
        rate_index++;
    }
    if (rate_index > 0) { // Make sure we don't underflow
        rate_index -= 1;
    }
    assert(rate_index < dash_rates().size());
    return rate_index;
}

template <MK_MOCK_AS(http::request_send, http_request_send),
          MK_MOCK_AS(http::request_recv_response, http_request_recv_response)>
void run_loop_(Var<net::Transport> txp, int speed_kbit, std::string auth_token,
               std::string uuid, Var<report::Entry> entry, Settings settings,
               Var<Reactor> reactor, Var<Logger> logger, Callback<Error> cb,
               int iteration = 1, Var<double> time_budget = nullptr) {

    if (!time_budget) {
        time_budget.reset(new double{0.0});
    }

    ErrorOr<bool> fast_scale_down = settings.get_noexcept(
        "fast_scale_down", false);
    if (!fast_scale_down) {
        logger->warn("dash: cannot parse `fast_scale_down' option");
        cb(fast_scale_down.as_error());
        return;
    }

    if (iteration > DASH_MAX_ITERATIONS) {
        logger->debug("dash: completed all iterations");
        cb(NoError());
        return;
    }

    /*
     * Select the rate that is lower than the latest measured speed and
     * compute the number of bytes to download such that downloading with
     * the selected rate takes DASH_SECONDS (in theory).
     */
    //size_t rate_index = select_lower_rate_index(speed_kbit);
    //int rate_kbit = dash_rates()[rate_index];
    int rate_kbit = speed_kbit; // XXX
    int count = ((rate_kbit * 1000) / 8) * DASH_SECONDS;

    std::string path = "/dash/download/";
    path += std::to_string(count);
    settings["http/path"] = path;
    settings["http/method"] = "GET";
    logger->debug("dash: requesting '%s'", path.c_str());

    // XXX
    double saved_time = mk::time_now();
    http_request_send(
        txp, settings,
        {
            {"Authorization", auth_token},
        },
        "", [=](Error error, Var<http::Request> req) {
            if (error) {
                logger->warn("dash: request failed: %s",
                             error.explain().c_str());
                cb(error);
                return;
            }
            assert(!!req);
            http_request_recv_response(
                txp,
                [=](Error error, Var<http::Response> res) {
                    if (error) {
                        logger->warn("dash: cannot receive response: %s",
                                     error.explain().c_str());
                        cb(error);
                        return;
                    }
                    assert(!!res);
                    if (res->status_code != 200) {
                        logger->warn("dash: invalid response code: %s",
                                     error.explain().c_str());
                        cb(http::HttpRequestFailedError());
                        return;
                    }
                    res->request = req;
                    double length = res->body.length();
                    double time_elapsed = mk::time_now() - saved_time;
                    if (time_elapsed <= 0) { // For robustness
                        logger->warn("dash: invalid time error");
                        cb(InvalidTimeError());
                        return;
                    }
                    // TODO: we should fill all the required fields
                    (*entry)["receiver_data"].push_back(report::Entry{
                        {"connect_time", txp->connect_time()},
                        //{"delta_user_time", delta_user_time}
                        //{"delta_sys_time", delta_sys_time}
                        {"elapsed", time_elapsed},
                        {"elapsed_target", DASH_SECONDS},
                        {"fast_scale_down", *fast_scale_down},
                        //{"internal_address", stream.myname[0]}
                        {"iteration", iteration},
                        //{"platform", sys.platform}
                        {"rate", rate_kbit},
                        //{"real_address", self.parent.real_address}
                        //{"received", received}
                        //{"remote_address", stream.peername[0]}
                        //{"request_ticks", self.saved_ticks}
                        {"timestamp", mk::time_now()},
                        {"uuid", uuid},
                        {"engine_name", "libmeasurement_kit"},
                        {"engine_version", MK_VERSION},
                        {"version", "0.006000000"}});
                    *time_budget += DASH_SECONDS - time_elapsed;
                    double speed = length / time_elapsed;
                    double s_k = (speed * 8) / 1000;
                    std::stringstream ss;
                    ss << "rate: " << rate_kbit << " kbit/s, speed: "
                       << std::fixed << std::setprecision(2) << s_k
                       << " kbit/s, elapsed: " << std::fixed
                       << std::setprecision(2) << time_elapsed
                       << "s, time budget: " << std::fixed
                       << std::setprecision(2) << *time_budget;
                    logger->progress(iteration / (double)DASH_MAX_ITERATIONS,
                                     ss.str().c_str());
                    if (*fast_scale_down == true &&
                          time_elapsed > DASH_SECONDS) {
                        // If the rate is too high, scale it down
                        double relerr = 1 - (time_elapsed / DASH_SECONDS);
                        s_k *= relerr;
                        if (s_k < 0) {
                            s_k = dash_rates()[0];
                        }
                    }
                    reactor->call_soon([=]() {
                        run_loop_<http_request_send,
                                  http_request_recv_response>(
                            txp, s_k, auth_token, uuid, entry, settings,
                            reactor, logger, cb, iteration + 1, time_budget);
                    });
                },
                reactor, logger);
        });
}

template <MK_MOCK_AS(http::request_connect, http_request_connect),
          MK_MOCK_AS(http::request_send, http_request_send),
          MK_MOCK_AS(http::request_recv_response, http_request_recv_response)>
void run_impl(std::string url, std::string auth_token, Var<report::Entry> entry,
              Settings settings, Var<Reactor> reactor, Var<Logger> logger,
              Callback<Error> cb) {
    settings["http/url"] = url;
    settings["http/method"] = "GET";
    logger->info("Start dash test with: %s", url.c_str());
    http_request_connect(
        settings,
        [=](Error error, Var<net::Transport> txp) {
            if (error) {
                assert(!txp);
                logger->warn("dash: cannot connect to server: %s",
                             error.explain().c_str());
                cb(error);
                return;
            }
            // Note: from now on, we own `txp`
            assert(!!txp);
            logger->info("Connected to server; starting the test");
            /*
             * TODO: from the Neubot point of view, it would probably be
             * very useful to save and reuse the same UUID4.
             */
            run_loop_<http_request_send, http_request_recv_response>(
                txp, dash_rates()[0], auth_token, mk::sole::uuid4().str(),
                entry, settings, reactor, logger, [=](Error error) {
                    // Release the `txp` before continuing
                    logger->info("Test complete; closing connection");
                    txp->close([=]() { cb(error); });
                });
        },
        reactor, logger);
}

/*
 * TODO: this code currently is specifically tailored to the DASH test and
 * we should instead make it more general and able to work with all the tests
 * implemented by Neubot (probably using a Continuation).
 */
template <MK_MOCK_AS(http::request_sendrecv, http_request_sendrecv)>
void negotiate_loop_(Var<report::Entry> entry, Var<net::Transport> txp,
                     Settings settings, Var<Reactor> reactor,
                     Var<Logger> logger, Callback<Error, std::string> callback,
                     int iteration = 0, std::string auth_token = "") {
    report::Entry value = {{"dash_rates", dash_rates()}};
    std::string body = value.dump();
    settings["http/path"] = "/negotiate/dash";
    settings["http/method"] = "POST";
    if (iteration > MAX_NEGOTIATIONS) {
        logger->warn("neubot: too many negotiations");
        callback(TooManyNegotiationsError(), "");
        return;
    }
    http_request_sendrecv(
        txp, settings,
        {
            {"Content-Type", "application/json"}, {"Authorization", auth_token},
        },
        body,
        [=](Error error, Var<http::Response> res) {
            if (error) {
                logger->warn("neubot: negotiate failed: %s",
                             error.explain().c_str());
                callback(error, "");
                return;
            }
            assert(!!res);
            if (res->status_code != 200) {
                logger->warn("neubot: negotiate failed on server side");
                callback(http::HttpRequestFailedError(), "");
                return;
            }
            // TODO: Here we can probably use the JSON specific API
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
            } catch (const std::exception &exc) {
                logger->warn("neubot: cannot parse negotiate response: %s",
                             exc.what());
                callback(CannotParseNegotiateResponseError(), "");
                return;
            }
            logger->debug("negotiation: unchoked=%d queue_pos=%d", unchoked,
                          queue_pos);
            if (!unchoked) {
                reactor->call_soon([=]() {
                    negotiate_loop_(entry, txp, settings, reactor, logger,
                                    callback, iteration + 1, auth);
                });
                return;
            }
            callback(NoError(), auth);
        },
        reactor, logger);
}

template <MK_MOCK_AS(http::request_sendrecv, http_request_sendrecv)>
void collect_(Var<net::Transport> txp, Var<report::Entry> entry,
              std::string auth, Settings settings, Var<Reactor> reactor,
              Var<Logger> logger, Callback<Error> cb) {
    // TODO: Here we can probably use the JSON specific API
    std::string body = (*entry)["receiver_data"].dump();
    logger->debug("Body sent to server: %s", body.c_str());
    settings["http/method"] = "POST";
    settings["http/path"] = "/collect/dash";
    http_request_sendrecv(
        txp, settings,
        {
            {"Content-Type", "application/json"}, {"Authorization", auth},
        },
        body,
        [=](Error error, Var<http::Response> res) {
            if (error) {
                logger->warn("neubot: collect failed: %s",
                             error.explain().c_str());
            } else {
                assert(!!res);
                if (res->status_code != 200) {
                    error = http::HttpRequestFailedError();
                }
            }
            logger->debug("Response received from server: %s",
                          res->body.c_str());
            cb(error);
        },
        reactor, logger);
}

template <MK_MOCK_AS(http::request_connect, http_request_connect),
          MK_MOCK_AS(http::request_sendrecv, http_request_sendrecv_negotiate),
          MK_MOCK_AS(http::request_sendrecv, http_request_sendrecv_collect)>
void negotiate_with_(std::string url, Var<report::Entry> entry,
                     Settings settings, Var<Reactor> reactor,
                     Var<Logger> logger, Callback<Error> cb) {
    logger->info("Negotiating with: %s", url.c_str());
    settings["http/url"] = url;
    http_request_connect(
        settings,
        [=](Error error, Var<net::Transport> txp) {
            if (error) {
                // Note: in this case we don't need to close the transport
                logger->warn("neubot: cannot connect to negotiate server: %s",
                             error.explain().c_str());
                cb(error);
                return;
            }
            // Note: from now on, we own the `txp` transport
            assert(!!txp);
            negotiate_loop_<http_request_sendrecv_negotiate>(
                entry, txp, settings, reactor, logger,
                [=](Error error, std::string auth_token) {
                    if (error) {
                        logger->warn("neubot: negotiate failed: %s",
                                     error.explain().c_str());
                        txp->close([=]() { cb(error); });
                        return;
                    }
                    // TODO: generalize code, allow to run more tests
                    run_impl(url, auth_token, entry, settings, reactor, logger,
                             [=](Error error) {
                                 if (error) {
                                     logger->warn("neubot: test failed: %s",
                                                  error.explain().c_str());
                                     /* FALLTHROUGH */
                                 }
                                 logger->info("Collecting results");
                                 collect_<http_request_sendrecv_collect>(
                                     txp, entry, auth_token, settings, reactor,
                                     logger, [=](Error error) {
                                         // Dispose of the `txp`
                                         txp->close([=]() { cb(error); });
                                     });
                             });
                });
        },
        reactor, logger);
}

template <MK_MOCK_AS(mlabns::query, mlabns_query)>
void negotiate_impl(Var<report::Entry> entry, Settings settings,
                    Var<Reactor> reactor, Var<Logger> logger,
                    Callback<Error> cb) {
    if (settings.find("url") != settings.end()) {
        negotiate_with_(settings["url"], entry, settings, reactor, logger, cb);
        return;
    }
    logger->info("Discovering mlab server using mlabns");
    mlabns_query("neubot",
                 [=](Error error, mlabns::Reply reply) {
                     if (error) {
                         logger->warn("neubot: mlabns error: %s",
                                      error.explain().c_str());
                         cb(error);
                         return;
                     }
                     negotiate_with_(reply.url, entry, settings, reactor,
                                     logger, cb);
                 },
                 settings, reactor, logger);
}

} // namespace dash
} // namespace neubot
} // namespace mk
#endif
