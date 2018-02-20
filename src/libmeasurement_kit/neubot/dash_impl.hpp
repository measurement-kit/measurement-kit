// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_NEUBOT_DASH_IMPL_HPP
#define SRC_LIBMEASUREMENT_KIT_NEUBOT_DASH_IMPL_HPP

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
 *
 * 6. As pointed out by a comment in the code, when we don't use a fixed
 *    vector or rates, the initial speed estimate is 3,000 kbit/s. This
 *    initial value was chosen because Netflix recommends to have at least
 *    that bandwidth to stream in SD quality. (Bandwidth is, of course,
 *    different from the bitrate, but a number close to 3,000 kbit/s
 *    seems anyway to be a reasonable starting point.)
 */

#include "src/libmeasurement_kit/neubot/dash.hpp"

#include "src/libmeasurement_kit/common/mock.hpp"
#include "src/libmeasurement_kit/common/utils.hpp"
#include "src/libmeasurement_kit/ext/sole.hpp"
#include "src/libmeasurement_kit/mlabns/mlabns.hpp"

#include <measurement_kit/common/json.hpp>
#include <measurement_kit/http.hpp>

#define DASH_INITIAL_RATE 3000
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
    int speed_kbit = -1; // Means: determine best initial value
    SharedPtr<report::Entry> entry;
    int iteration = 1;
    SharedPtr<Logger> logger;
    SharedPtr<Reactor> reactor;
    std::string real_address;
    Settings settings;
    SharedPtr<net::Transport> txp;
    std::string uuid;
};

template <MK_MOCK_AS(http::request_send, http_request_send),
          MK_MOCK_AS(http::request_recv_response, http_request_recv_response)>
void run_loop_(SharedPtr<DashLoopCtx> ctx) {
    // TODO: We may want to move this parsing of options in the setup
    // phase of the test and store them inside `ctx`.
    ErrorOr<bool> fast_scale_down =
          ctx->settings.get_noexcept("fast_scale_down", false);
    if (!fast_scale_down) {
        ctx->logger->warn("dash: cannot parse `fast_scale_down' option");
        ctx->cb(fast_scale_down.as_error());
        return;
    }
    ErrorOr<int> constant_bitrate =
          ctx->settings.get_noexcept("constant_bitrate", 0);
    if (!constant_bitrate || *constant_bitrate < 0) {
        ctx->logger->warn("dash: cannot parse `constant_bitrate' option");
        ctx->cb(ValueError());
        return;
    }
    ErrorOr<bool> use_fixed_rates =
          ctx->settings.get_noexcept("use_fixed_rates", false);
    if (!use_fixed_rates) {
        ctx->logger->warn("dash: cannot parse `use_fixed_rates' option");
        ctx->cb(use_fixed_rates.as_error());
        return;
    }
    ErrorOr<int> elapsed_target =
          ctx->settings.get_noexcept("elapsed_target", DASH_SECONDS);
    if (!elapsed_target || *elapsed_target < 0) {
        ctx->logger->warn("dash: cannot parse `elapsed_target' option");
        ctx->cb(ValueError());
        return;
    }
    ErrorOr<int> max_iterations =
          ctx->settings.get_noexcept("max_iteration", DASH_MAX_ITERATIONS);
    if (!max_iterations || *max_iterations < 0) {
        ctx->logger->warn("dash: cannot parse `max_iteration' option");
        ctx->cb(ValueError());
        return;
    }
    ErrorOr<int> initial_rate =
          ctx->settings.get_noexcept("initial_rate", DASH_INITIAL_RATE);
    if (!initial_rate || *initial_rate < 0) {
        ctx->logger->warn("dash: cannot parse `initial_rate' option");
        ctx->cb(ValueError());
        return;
    }

    if (ctx->iteration > *max_iterations) {
        ctx->logger->debug("dash: completed all iterations");
        try {
            std::vector<double> rates;
            std::vector<double> stalls;
            double frame_ready_time = 0.0;
            double play_time = 0.0;
            double connect_latency = 0.0;
            for (auto &e : (*ctx->entry)["receiver_data"]) {
                if (connect_latency == 0.0) {
                    // It is always equal for all the records
                    connect_latency = e["connect_time"];
                }
                rates.push_back(e["rate"]);
                /* The first chunk is played when it arrives. To have smooth
                   video, we'd like to play each subsequent chunk within
                   `elapsed_target` seconds. So, the player has always something
                   to play and the user sees the video. If a chunk arrives
                   earlier than the play deadline, good because we can request
                   the next chunk also earlier. That is, we increase buffer
                   time for slow delivery. On the contrary, if a chunk arrives
                   later than the play deadline, we need to stop playing. The
                   max(stalls) is the delay we would have needed to add at
                   the beginning to make sure we had no player stalls. */
                double elapsed = e["elapsed"];
                frame_ready_time += elapsed;
                double elapsed_target = e["elapsed_target"];
                // Note: this says that the play time of the first frame is
                // when we receive it. Subsequent frames must be played after
                // `elapsed_target` seconds each to have smooth video.
                play_time +=
                      (play_time == 0) ? frame_ready_time : elapsed_target;
                double stall = frame_ready_time - play_time;
                stalls.push_back(stall);
            }
            (*ctx->entry)["simple"]["connect_latency"] = connect_latency;
            (*ctx->entry)["simple"]["median_bitrate"] = mk::median(rates);
            (*ctx->entry)["simple"]["min_playout_delay"] =
                  (stalls.size() > 0)
                        ? *std::max_element(stalls.begin(), stalls.end())
                        : 0.0;
        } catch (...) {
            ctx->logger->warn("dash: cannot save summary information");
        }
        ctx->cb(NoError());
        return;
    }
    if (ctx->speed_kbit < 0) {
        // Determine initial speed estimate. In legacy mode (i.e. when we use
        // a fixed vector of rates), we use the first entry. Otherwise, we use
        // as initial speed estimate 3000 kbit/s. This number is the minimum
        // speed recommended by Netflix to stream in SD quality, as of 13 July
        // 2017. I though this would be a good starting point.
        //
        // See: <https://help.netflix.com/en/node/306>.
        ctx->speed_kbit = (*use_fixed_rates == true)
              ? dash_rates()[0] : *initial_rate;
    }
    /*
     * Select the rate that is lower than the latest measured speed and
     * compute the number of bytes to download such that downloading with
     * the selected rate takes `elapsed_target` (in theory).
     */
    int rate_kbit =
          (*use_fixed_rates == true)
                ? dash_rates()[select_lower_rate_index(ctx->speed_kbit)]
                : (*constant_bitrate > 0) ? *constant_bitrate : ctx->speed_kbit;
    int count = ((rate_kbit * 1000) / 8) * (*elapsed_target);
    std::string path = "/dash/download/";
    path += std::to_string(count);
    Settings settings = ctx->settings; /* Make a local copy */
    settings["http/ignore_body"] = true;
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
                {"Cache-Control", "no-cache, no-store, must-revalidate"},
          },
          "", ctx->logger, [=](Error error, SharedPtr<http::Request> req) {
              if (error) {
                  ctx->logger->warn("dash: request failed: %s", error.what());
                  ctx->cb(error);
                  return;
              }
              assert(!!req);
              http_request_recv_response(
                    ctx->txp,
                    [=](Error error, SharedPtr<http::Response> res) {
                        if (error) {
                            ctx->logger->warn(
                                  "dash: cannot receive response: %s",
                                  error.what());
                            ctx->cb(error);
                            return;
                        }
                        assert(!!res);
                        if (res->status_code != 200) {
                            ctx->logger->warn("dash: invalid response code: %s",
                                              error.what());
                            ctx->cb(http::HttpRequestFailedError());
                            return;
                        }
                        /*
                         * XXX: This test assumes that HTTP caches are not
                         * closing the connection after each request. But there
                         * are networks in which this happens, as documented
                         * in measurement-kit/measurement-kit#1322. In such case
                         * what we do is that we abort the test.
                         */
                        if (res->headers["connection"] == "close") {
                            ctx->logger->warn("dash: middlebox detected error");
                            ctx->cb(MiddleboxDetectedError());
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
                              {"constant_bitrate", *constant_bitrate != 0},
                              {"delta_user_time", 0.0},
                              {"delta_sys_time", 0.0},
                              {"elapsed", time_elapsed},
                              {"elapsed_target", *elapsed_target},
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
                           << " kbit/s, elapsed: " << time_elapsed << " s";
                        ctx->logger->progress(ctx->iteration /
                                                    (double)*max_iterations,
                                              ss.str().c_str());
                        if (*fast_scale_down == true &&
                            time_elapsed > *elapsed_target) {
                            // If the rate is too high, scale it down
                            double relerr =
                                  1 - (time_elapsed / *elapsed_target);
                            s_k *= relerr;
                            if (s_k <= 0) {
                                s_k = dash_rates()[0];
                            }
                        }
                        ctx->speed_kbit = (int)s_k;
                        ctx->iteration += 1;
                        run_loop_<http_request_send,
                                  http_request_recv_response>(ctx);
                    },
                    ctx->settings, ctx->reactor, ctx->logger);
          });
}

template <MK_MOCK_AS(http::request_connect, http_request_connect),
          MK_MOCK_AS(http::request_send, http_request_send),
          MK_MOCK_AS(http::request_recv_response, http_request_recv_response)>
void run_impl(std::string url, std::string auth_token, std::string real_address,
              SharedPtr<report::Entry> entry, Settings settings, SharedPtr<Reactor> reactor,
              SharedPtr<Logger> logger, Callback<Error> cb) {
    SharedPtr<DashLoopCtx> ctx = SharedPtr<DashLoopCtx>::make();
    ctx->auth_token = auth_token;
    ctx->entry = entry;
    ctx->logger = logger;
    ctx->reactor = reactor;
    ctx->real_address = real_address;
    ctx->settings = settings;
    //
    // Neubot used to generate a random UUID for the probe and to keep it
    // consistent over time to enable time series analyses. The problem of
    // doing that on mobile is that it will most likely allow to identify
    // and/or track people. For this reason, the code for allowing setting
    // a specific UUID has been removed and we generate a random new UUID
    // each time we run a new DASH test.
    //
    ctx->uuid = mk::sole::uuid4().str();
    settings["http/url"] = url;
    settings["http/method"] = "GET";
    logger->info("Start dash test with: %s", url.c_str());
    http_request_connect(
          settings,
          [=](Error error, SharedPtr<net::Transport> txp) {
              if (error) {
                  logger->warn("dash: cannot connect to server: %s",
                               error.what());
                  cb(error);
                  return;
              }
              // Note: from now on, we own `txp`
              assert(!!txp);
              logger->info("Connected to server (3WHS RTT = %f s); starting "
                           "the test", txp->connect_time());
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
void negotiate_loop_(SharedPtr<report::Entry> entry, SharedPtr<net::Transport> txp,
                     Settings settings, SharedPtr<Reactor> reactor,
                     SharedPtr<Logger> logger,
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
                {"Cache-Control", "no-cache, no-store, must-revalidate"},
          },
          body,
          [=](Error error, SharedPtr<http::Response> res) {
              if (error) {
                  logger->warn("neubot: negotiate failed: %s", error.what());
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
              error = json_process(
                    res->body, [&](Json &respbody) {
                        auth = respbody.at("authorization");
                        queue_pos = respbody.at("queue_pos");
                        real_address = respbody.at("real_address");
                        unchoked = respbody.at("unchoked");
                    });
              if (error) {
                  logger->warn("neubot: cannot parse negotiate response: %s",
                               error.what());
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
void collect_(SharedPtr<net::Transport> txp, SharedPtr<report::Entry> entry,
              std::string auth, Settings settings, SharedPtr<Reactor> reactor,
              SharedPtr<Logger> logger, Callback<Error> cb) {
    std::string body = (*entry)["receiver_data"].dump();
    logger->debug("Body sent to server: %s", body.c_str());
    settings["http/method"] = "POST";
    settings["http/path"] = "/collect/dash";
    http_request_sendrecv(
          txp, settings,
          {
                {"Content-Type", "application/json"}, {"Authorization", auth},
                {"Cache-Control", "no-cache, no-store, must-revalidate"},
          },
          body,
          [=](Error error, SharedPtr<http::Response> res) {
              if (error) {
                  logger->warn("neubot: collect failed: %s", error.what());
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
              error = json_process(
                    res->body, [&](Json &json) {
                        (*entry)["sender_data"] = json;
                    });
              cb(error);
          },
          reactor, logger);
}

template <MK_MOCK_AS(http::request_connect, http_request_connect),
          MK_MOCK_AS(http::request_sendrecv, http_request_sendrecv_negotiate),
          MK_MOCK_AS(http::request_sendrecv, http_request_sendrecv_collect)>
void negotiate_with_(std::string hostname, SharedPtr<report::Entry> entry,
                     Settings settings, SharedPtr<Reactor> reactor,
                     SharedPtr<Logger> logger, Callback<Error> cb) {
    logger->info("Negotiating with: %s", hostname.c_str());
    std::stringstream ss;
    ss << "http://" << hostname << "/";
    std::string url = ss.str();
    settings["http/url"] = url;
    http_request_connect(
          settings,
          [=](Error error, SharedPtr<net::Transport> txp) {
              if (error) {
                  // Note: in this case we don't need to close the transport
                  // because we get passed a dumb transport on error
                  logger->warn("neubot: cannot connect to negotiate server: %s",
                               error.what());
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
                                         error.what());
                            txp->close([=]() { cb(error); });
                            return;
                        }
                        // TODO: generalize code, allow to run more tests
                        run_impl(url, auth_token, real_address, entry, settings,
                                 reactor, logger, [=](Error error) {
                                     if (error) {
                                         logger->warn("neubot: test failed: %s",
                                                      error.what());
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
void negotiate_impl(SharedPtr<report::Entry> entry, Settings settings,
                    SharedPtr<Reactor> reactor, SharedPtr<Logger> logger,
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
                                      error.what());
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
