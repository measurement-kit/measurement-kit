// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef PRIVATE_NEUBOT_DASH_IMPL_HPP
#define PRIVATE_NEUBOT_DASH_IMPL_HPP

/*
 * Here we implement the measurement methodology described in "Measuring
 * DASH Streaming Performance from the End Users Perspective using Neubot",
 * by S. Basso, A. Servetti, E. Masala, J.C. De Martin.
 *
 * See: https://nexa.polito.it/publications/basso2014measuring
 *
 * Differences from the methodology outlined in the paper:
 *
 * 1. By default, we don't scale down significantly the speed after we
 *    sense some queue in the network. That was meant to avoid clogging
 *    the downlink for Neubot (a background tool), but for OONI we do
 *    actually plan to run the test in the foreground. To re-enable the
 *    old behavior, set the `fast_scale_down` option to non-zero.
 *
 * 2. By default, we don't use the vector of fixed rates but rather we
 *    use as rate the speed computed in the previous set. This should
 *    yield a more accurate streaming speed estimate. This different
 *    behavior was suggested to me by Antonio Servetti sometime around
 *    May '17. To re-enable the old behavior, you need to set the
 *    `use_fixed_rates` option to non-zero.
 *
 * 3. As pointed out also by a comment in the code, our measurement of
 *    the download speed starts when we send the request rather when we
 *    receive the first byte of the response. I think this is not bad,
 *    since it actually captures better the time to fetch the chunk that
 *    one should play, as opposed to accurately estimating the speed.
 *
 * 4. We leave to zero the `delta_user_time` and `delta_sys_time`. We
 *    may change this in the future, when we understand how to get them
 *    portably. They were not so important for us anyway.
 *
 * 5. As pointed out also by a comment in the code, we don't consider
 *    the overhead of protocols when computing the speed. As for point
 *    three above, this allows to write a simpler implementation than
 *    the one in Neubot and, as a byproduct, provides us with information
 *    closer to know how much time it takes to get a playable chunk
 *    than just measuring the raw speed (I think, when I wrote the test
 *    for Neubot, I had too much bias towards measuring speed).
 */

#include "private/common/json.hpp"
#include "private/common/mock.hpp"
#include "private/common/utils.hpp"
#include <measurement_kit/ext/sole.hpp>
#include <measurement_kit/http.hpp>
#include <measurement_kit/mlabns.hpp>
#include <measurement_kit/neubot.hpp>

#define DASH_MAX_ITERATIONS 15
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

class DashLoopCtx {
  public:
    std::string auth_token;
    Callback<Error> cb;
    int speed_kbit = dash_rates()[0];
    Var<report::Entry> entry;
    int iteration = 1;
    Var<Logger> logger;
    Var<Reactor> reactor;
    std::string real_address;
    Settings settings;
    Var<net::Transport> txp;
    std::string uuid;
};

template <MK_MOCK_AS(http::request_send, http_request_send),
          MK_MOCK_AS(http::request_recv_response, http_request_recv_response)>
void run_loop_(Var<DashLoopCtx> ctx) {
    // TODO: We may want to move this parsing of options in the setup
    // phase of the test and store them inside `ctx`.
    ErrorOr<bool> fast_scale_down =
          ctx->settings.get_noexcept("fast_scale_down", false);
    if (!fast_scale_down) {
        ctx->logger->warn("dash: cannot parse `fast_scale_down' option");
        ctx->cb(fast_scale_down.as_error());
        return;
    }
    ErrorOr<bool> use_fixed_rates =
          ctx->settings.get_noexcept("use_fixed_rates", false);
    if (!use_fixed_rates) {
        ctx->logger->warn("dash: cannot parse `use_fixed_rates' option");
        ctx->cb(use_fixed_rates.as_error());
        return;
    }
    if (ctx->iteration > DASH_MAX_ITERATIONS) {
        ctx->logger->debug("dash: completed all iterations");
        ctx->cb(NoError());
        return;
    }
    /*
     * Select the rate that is lower than the latest measured speed and
     * compute the number of bytes to download such that downloading with
     * the selected rate takes DASH_SECONDS (in theory).
     */
    int rate_kbit =
          (*use_fixed_rates == true)
                ? dash_rates()[select_lower_rate_index(ctx->speed_kbit)]
                : ctx->speed_kbit;
    int count = ((rate_kbit * 1000) / 8) * DASH_SECONDS;
    std::string path = "/dash/download/";
    path += std::to_string(count);
    Settings settings = ctx->settings; /* Make a local copy */
    settings["http/path"] = path;
    settings["http/method"] = "GET";
    ctx->logger->debug("dash: requesting '%s'", path.c_str());
    /*
     * Note: our accounting of time also includes the time to send the
     * request to the server (approximately one RTT).
     *
     * This is different from the original implementation of DASH that
     * is part of Neubot.
     */
    double saved_time = mk::time_now();
    http_request_send(
          ctx->txp, settings,
          {
                {"Authorization", ctx->auth_token},
          },
          "", [=](Error error, Var<http::Request> req) {
              if (error) {
                  ctx->logger->warn("dash: request failed: %s",
                                    error.explain().c_str());
                  ctx->cb(error);
                  return;
              }
              assert(!!req);
              http_request_recv_response(
                    ctx->txp,
                    [=](Error error, Var<http::Response> res) {
                        if (error) {
                            ctx->logger->warn(
                                  "dash: cannot receive response: %s",
                                  error.explain().c_str());
                            ctx->cb(error);
                            return;
                        }
                        assert(!!res);
                        if (res->status_code != 200) {
                            ctx->logger->warn("dash: invalid response code: %s",
                                              error.explain().c_str());
                            ctx->cb(http::HttpRequestFailedError());
                            return;
                        }
                        res->request = req;
                        auto length = res->body.length();
                        double time_elapsed = mk::time_now() - saved_time;
                        if (time_elapsed <= 0) { // For robustness
                            ctx->logger->warn("dash: negative time error");
                            ctx->cb(GenericError("negative_time_error"));
                            return;
                        }
                        (*ctx->entry)["receiver_data"].push_back(report::Entry{
                              {"connect_time", ctx->txp->connect_time()},
                              {"delta_user_time", 0.0},
                              {"delta_sys_time", 0.0},
                              {"elapsed", time_elapsed},
                              {"elapsed_target", DASH_SECONDS},
                              {"engine_name", "libmeasurement_kit"},
                              {"engine_version", MK_VERSION},
                              {"fast_scale_down", *fast_scale_down},
                              {"internal_address",
                               ctx->txp->sockname().hostname},
                              {"iteration", ctx->iteration},
                              {"platform", mk_platform()},
                              {"rate", rate_kbit},
                              {"real_address", ctx->real_address},
                              /*
                               * Note: here we're only concerned with the amount
                               * of useful data we received (we ignore overhead)
                               *
                               * This is different from the original
                               * implementation of DASH that is part of Neubot.
                               */
                              {"received", length},
                              {"remote_address", ctx->txp->peername().hostname},
                              {"request_ticks", saved_time},
                              {"timestamp", llround(saved_time)},
                              {"use_fixed_rates", *use_fixed_rates},
                              {"uuid", ctx->uuid},
                              /*
                               * This version indicates measurement-kit.
                               */
                              {"version", "0.007000000"}});
                        double speed = length / time_elapsed;
                        double s_k = (speed * 8) / 1000;
                        std::stringstream ss;
                        ss << "rate: " << rate_kbit
                           << " kbit/s, speed: " << std::fixed
                           << std::setprecision(2) << s_k
                           << " kbit/s, elapsed: " << time_elapsed << "s";
                        ctx->logger->progress(ctx->iteration /
                                                    (double)DASH_MAX_ITERATIONS,
                                              ss.str().c_str());
                        if (*fast_scale_down == true &&
                            time_elapsed > DASH_SECONDS) {
                            // If the rate is too high, scale it down
                            double relerr = 1 - (time_elapsed / DASH_SECONDS);
                            s_k *= relerr;
                            if (s_k <= 0) {
                                s_k = dash_rates()[0];
                            }
                        }
                        ctx->speed_kbit = s_k;
                        ctx->iteration += 1;
                        run_loop_<http_request_send,
                                  http_request_recv_response>(ctx);
                    },
                    ctx->reactor, ctx->logger);
          });
}

template <MK_MOCK_AS(http::request_connect, http_request_connect),
          MK_MOCK_AS(http::request_send, http_request_send),
          MK_MOCK_AS(http::request_recv_response, http_request_recv_response)>
void run_impl(std::string url, std::string auth_token, std::string real_address,
              Var<report::Entry> entry, Settings settings, Var<Reactor> reactor,
              Var<Logger> logger, Callback<Error> cb) {
    Var<DashLoopCtx> ctx = Var<DashLoopCtx>::make();
    ctx->auth_token = auth_token;
    ctx->entry = entry;
    ctx->logger = logger;
    ctx->reactor = reactor;
    ctx->real_address = real_address;
    ctx->settings = settings;
    /*
     * From the Neubot point of view, it is for sure very very useful to save
     * and reuse the same UUID4. We can attempt to do that by asking the caller
     * to pass us the unique identifier to be used for this client.
     */
    if (ctx->settings.find("uuid") == ctx->settings.end()) {
        ctx->logger->warn("You passed me no UUID, generating a random one");
        ctx->uuid = mk::sole::uuid4().str();
    } else {
        ctx->uuid = ctx->settings.at("uuid");
    }
    settings["http/url"] = url;
    settings["http/method"] = "GET";
    logger->info("Start dash test with: %s", url.c_str());
    http_request_connect(
          settings,
          [=](Error error, Var<net::Transport> txp) {
              if (error) {
                  logger->warn("dash: cannot connect to server: %s",
                               error.explain().c_str());
                  cb(error);
                  return;
              }
              // Note: from now on, we own `txp`
              assert(!!txp);
              logger->info("Connected to server; starting the test");
              ctx->txp = txp;
              ctx->cb = [=](Error error) {
                  // Release the `txp` before continuing
                  logger->info("Test complete; closing connection");
                  txp->close([=]() { cb(error); });
              };
              run_loop_<http_request_send, http_request_recv_response>(ctx);
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
                     Var<Logger> logger,
                     Callback<Error, std::string, std::string> callback,
                     int iteration = 0, std::string auth_token = "") {
    report::Entry value = {{"dash_rates", dash_rates()}};
    std::string body = value.dump();
    settings["http/path"] = "/negotiate/dash";
    settings["http/method"] = "POST";
    if (iteration > MAX_NEGOTIATIONS) {
        logger->warn("neubot: too many negotiations");
        callback(GenericError("too_many_negotiations"), "", "");
        return;
    }
    http_request_sendrecv(
          txp, settings,
          {
                {"Content-Type", "application/json"},
                {"Authorization", auth_token},
          },
          body,
          [=](Error error, Var<http::Response> res) {
              if (error) {
                  logger->warn("neubot: negotiate failed: %s",
                               error.explain().c_str());
                  callback(error, "", "");
                  return;
              }
              assert(!!res);
              if (res->status_code != 200) {
                  logger->warn("neubot: negotiate failed on server side");
                  callback(http::HttpRequestFailedError(), "", "");
                  return;
              }
              std::string auth;
              int queue_pos = 0;
              std::string real_address;
              int unchoked = 0;
              error = json_parse_process_and_filter_errors(
                    res->body, [&](nlohmann::json &respbody) {
                        auth = respbody.at("authorization");
                        queue_pos = respbody.at("queue_pos");
                        real_address = respbody.at("real_address");
                        unchoked = respbody.at("unchoked");
                    });
              if (error) {
                  logger->warn("neubot: cannot parse negotiate response: %s",
                               error.as_ooni_error().c_str());
                  callback(error, "", "");
                  return;
              }
              logger->debug("negotiation: unchoked=%d queue_pos=%d", unchoked,
                            queue_pos);
              if (!unchoked) {
                  negotiate_loop_(entry, txp, settings, reactor, logger,
                                  callback, iteration + 1, auth);
                  return;
              }
              callback(NoError(), auth, real_address);
          },
          reactor, logger);
}

template <MK_MOCK_AS(http::request_sendrecv, http_request_sendrecv)>
void collect_(Var<net::Transport> txp, Var<report::Entry> entry,
              std::string auth, Settings settings, Var<Reactor> reactor,
              Var<Logger> logger, Callback<Error> cb) {
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
                               error.as_ooni_error().c_str());
                  cb(error);
                  return;
              }
              assert(!!res);
              if (res->status_code != 200) {
                  cb(http::HttpRequestFailedError());
                  return;
              }
              logger->debug("Response received from server: %s",
                            res->body.c_str());
              error = json_parse_process_and_filter_errors(
                    res->body, [&](nlohmann::json &json) {
                        (*entry)["sender_data"] = json;
                    });
              cb(error);
          },
          reactor, logger);
}

template <MK_MOCK_AS(http::request_connect, http_request_connect),
          MK_MOCK_AS(http::request_sendrecv, http_request_sendrecv_negotiate),
          MK_MOCK_AS(http::request_sendrecv, http_request_sendrecv_collect)>
void negotiate_with_(std::string hostname, Var<report::Entry> entry,
                     Settings settings, Var<Reactor> reactor,
                     Var<Logger> logger, Callback<Error> cb) {
    logger->info("Negotiating with: %s", hostname.c_str());
    std::stringstream ss;
    ss << "http://" << hostname << "/";
    std::string url = ss.str();
    settings["http/url"] = url;
    http_request_connect(
          settings,
          [=](Error error, Var<net::Transport> txp) {
              if (error) {
                  // Note: in this case we don't need to close the transport
                  // because we get passed a dumb transport on error
                  logger->warn("neubot: cannot connect to negotiate server: %s",
                               error.explain().c_str());
                  cb(error);
                  return;
              }
              // Note: from now on, we own the `txp` transport
              assert(!!txp);
              negotiate_loop_<http_request_sendrecv_negotiate>(
                    entry, txp, settings, reactor, logger,
                    [=](Error error, std::string auth_token,
                        std::string real_address) {
                        if (error) {
                            logger->warn("neubot: negotiate failed: %s",
                                         error.explain().c_str());
                            txp->close([=]() { cb(error); });
                            return;
                        }
                        // TODO: generalize code, allow to run more tests
                        run_impl(url, auth_token, real_address, entry, settings,
                                 reactor, logger, [=](Error error) {
                                     if (error) {
                                         logger->warn("neubot: test failed: %s",
                                                      error.explain().c_str());
                                         /* FALLTHROUGH */
                                     }
                                     logger->info("Collecting results");
                                     collect_<http_request_sendrecv_collect>(
                                           txp, entry, auth_token, settings,
                                           reactor, logger, [=](Error error) {
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
    if (settings.find("hostname") != settings.end()) {
        negotiate_with_(settings["hostname"], entry, settings,
                        reactor, logger, cb);
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
                     negotiate_with_(reply.fqdn, entry, settings, reactor,
                                     logger, cb);
                 },
                 settings, reactor, logger);
}

} // namespace dash
} // namespace neubot
} // namespace mk
#endif
