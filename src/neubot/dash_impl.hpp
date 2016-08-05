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

#define DASH_MAX_ITERATION 15
#define DASH_SECONDS 2

using namespace mk;
using namespace mk::net;
using namespace mk::http;
using namespace mk::report;

namespace mk {
namespace neubot {
namespace dash {

template <MK_MOCK_NAMESPACE(http, request_send)>
void loop_request(Var<Transport> transport, int speed_kbit,
                  Callback<Error, Var<Entry>> cb, Var<Entry> measurements,
                  std::string auth, Settings settings, Var<Reactor> reactor,
                  Var<Logger> logger, int iteration = 1) {

    int rate_index = 0;
    static const int DASH_RATES[] = {100,  150,  200,  250,  300,   400,  500,
                                     700,  900,  1200, 1500, 2000,  2500, 3000,
                                     4000, 5000, 6000, 7000, 10000, 20000};
    static const int sizeofrates = (int)(sizeof(DASH_RATES) / sizeof(int));
    std::string path = "/dash/download/";

    if (iteration > DASH_MAX_ITERATION) {
        transport->close([=]() { cb(NoError(), measurements); });
        return;
    }

    // Bisect
    while ((DASH_RATES[rate_index] < speed_kbit) &&
           (rate_index < sizeofrates)) {
        rate_index++;
    }
    rate_index -= 1;
    if (rate_index < 0) {
        rate_index = 0;
    }

    int rate_kbit = DASH_RATES[rate_index];
    int count = ((rate_kbit * 1000) / 8) * DASH_SECONDS;
    path += std::to_string(count);
    settings["http/path"] = path;
    double saved_times = mk::time_now();

    request_send(
        transport, settings,
        {
            {"Authorization", auth},
        },
        "",
        [=](Error error) {
            if (error) {
                logger->warn("Error: %d", (int)error);
                cb(error, nullptr);
                return;
            }

            request_recv_response(
                transport,
                [=](Error error, Var<Response> res) {
                    if (error) {
                        logger->warn("Error: %d", (int)error);
                        cb(error, nullptr);
                        return;
                    }

                    double new_times = mk::time_now();
                    float length = res->body.length();
                    double time_elapsed = new_times - saved_times;

                    if (time_elapsed < 0) {
                        logger->warn("Time elapsed can't be negative");
                        cb(NegativeTimeError(), nullptr);
                        return;
                    }
                    // TODO
                    if (auth != "") {
                        Entry result = {
                            //{"connect_time", self.rtts[0]}
                            //{"delta_user_time", delta_user_time}
                            //{"delta_sys_time", delta_sys_time}
                            {"elapsed", time_elapsed},        // OK
                            {"elapsed_target", DASH_SECONDS}, // OK
                            //{"internal_address", stream.myname[0]}
                            {"iteration", iteration}, // OK
                            //{"platform", sys.platform}
                            {"rate", rate_kbit}, // OK
                            //{"real_address", self.parent.real_address}
                            //{"received", received}
                            //{"remote_address", stream.peername[0]}
                            //{"request_ticks", self.saved_ticks}
                            //{"timestamp", mk::time_now}
                            //{"uuid", self.conf.get("uuid")}
                            {"version", MEASUREMENT_KIT_VERSION}};
                        measurements->push_back(result);
                    }

                    double speed = double(length / time_elapsed);
                    int s_k = (speed * 8) / 1000;

                    std::cout << "DASH: [" << iteration << "/"
                              << DASH_MAX_ITERATION << "] rate: " << rate_kbit
                              << " Kbit/s, speed: " << s_k
                              << " Kbit/s, elapsed: " << time_elapsed << " s\n";

                    if (time_elapsed > DASH_SECONDS) {
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

template <MK_MOCK_NAMESPACE(http, request_connect)>
void run_impl(Settings settings, Callback<Error, Var<Entry>> cb,
              std::string auth, Var<Reactor> reactor, Var<Logger> logger) {
    settings["http/path"] = "";
    settings["http/method"] = "GET";

    request_connect(settings,
                    [=](Error error, Var<Transport> transport) {

                        if (error) {
                            logger->warn("Error: %d", (int)error);
                            cb(error, nullptr);
                            return;
                        }

                        Var<Entry> measurements(new Entry);

                        loop_request(transport, 100, cb, measurements, auth,
                                     settings, reactor, logger);

                        return;
                    },
                    reactor, logger);
}

} // namespace dash
} // namespace neubot
} // namespace mk
