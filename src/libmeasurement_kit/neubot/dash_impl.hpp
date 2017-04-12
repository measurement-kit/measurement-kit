// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_NEUBOT_DASH_IMPL_HPP
#define SRC_LIBMEASUREMENT_KIT_NEUBOT_DASH_IMPL_HPP

#include "../common/utils.hpp"
#include "../neubot/utils.hpp"
#include <iostream>
#include <measurement_kit/http.hpp>
#include <measurement_kit/mlabns.hpp>
#include <measurement_kit/neubot.hpp>
#include <measurement_kit/report.hpp>
#include <stdlib.h>
#include <string>
#include <unistd.h>

#define DASH_MAX_ITERATION 15
#define DASH_SECONDS 2

using namespace mk;
using namespace mk::net;
using namespace mk::http;
using namespace mk::report;

namespace mk {
namespace neubot {
namespace dash {

// XXX: it makes no sense that we have measurements both as argument
// and as part of the callback
template <MK_MOCK_AS(http::request_send, http_request_send),
          MK_MOCK_AS(http::request_recv_response, http_request_recv_response)>
void loop_request(Var<Transport> transport, int speed_kbit,
                  Callback<Error, Var<Entry>> cb, Var<Entry> measurements,
                  std::string auth, Settings settings, Var<Reactor> reactor,
                  Var<Logger> logger, int iteration = 1) {
    if (iteration > DASH_MAX_ITERATION) {
        cb(NoError(), measurements);
        return;
    }

    // Select the rate index lower than the current speed
    size_t rate_index = 0;
    while (dash_rates()[rate_index] < speed_kbit &&
           rate_index < dash_rates().size()) {
        rate_index++;
    }
    if (rate_index > 0) { // Make sure we don't underflow
        rate_index -= 1;
    }

    std::string path = "/dash/download/";
    int rate_kbit = dash_rates()[rate_index];
    int count = ((rate_kbit * 1000) / 8) * DASH_SECONDS;
    path += std::to_string(count);
    settings["http/path"] = path;
    double saved_times = mk::time_now();

    http_request_send(
        transport, settings,
        {
            {"Authorization", auth},
        },
        "",
        [=](Error error, Var<Request> req) {
            if (error) {
                logger->warn("dash: request failed: %s",
                             error.explain().c_str());
                cb(error, measurements);
                return;
            }
            http_request_recv_response(
                transport,
                [=](Error error, Var<Response> res) {
                    if (error) {
                        logger->warn("dash: cannot receive response: %s",
                                     error.explain().c_str());
                        cb(error, measurements);
                        return;
                    }
                    res->request = req;
                    double new_times = mk::time_now();
                    float length = res->body.length();
                    double time_elapsed = new_times - saved_times;
                    if (time_elapsed < 0) { // For robustness
                        logger->warn("dash: elapsed time can't be negative");
                        cb(NegativeTimeError(), measurements);
                        return;
                    }
                    // TODO: we should fill all the required fields
                    Entry result = {
                        //{"connect_time", self.rtts[0]}
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
                        {"version", MEASUREMENT_KIT_VERSION}
                    };
                    measurements->push_back(result);
                    double speed = length / time_elapsed;
                    double s_k = (speed * 8) / 1000;
                    logger->debug("[%d/%d] rate: %d kbit/s, speed: %.2f "
                                  "kbit/s, elapsed: %.2f s", iteration,
                                  DASH_MAX_ITERATION, rate_kbit, s_k,
                                  time_elapsed);
                    if (time_elapsed > DASH_SECONDS) {
                        // If the rate is too high, scale it down
                        float relerr = 1 - (time_elapsed / DASH_SECONDS);
                        s_k *= relerr;
                        if (s_k < 0) {
                            s_k = 100;
                        }
                    }
                    reactor->call_soon([=]() {
                        loop_request(transport, s_k, cb, measurements, auth,
                                     settings, reactor, logger, iteration + 1);
                    });
                },
                reactor, logger);
        });
}

template <MK_MOCK_AS(http::request_connect, http_request_connect)>
void run_impl(Settings settings, Callback<Error, Var<Entry>> cb,
              std::string auth, Var<Reactor> reactor, Var<Logger> logger) {
    Var<Entry> measurements{new Entry};
    request_connect(settings,
                    [=](Error error, Var<Transport> transport) {
                        if (error) {
                            logger->warn("dash: cannot connect to server: %s",
                                         error.explain().c_str());
                            cb(error, measurements);
                            return;
                        }
                        loop_request(transport, 100,
                                     [=](Error e, Var<Entry>) {
                                         transport->close([=]() {
                                             // XXX As said above, it does not
                                             // make sense to pass measurements
                                             // around twice
                                             cb(e, measurements);
                                         });
                                     },
                                     measurements, auth, settings, reactor,
                                     logger);
                    },
                    reactor, logger);
}

} // namespace dash
} // namespace neubot
} // namespace mk
#endif
