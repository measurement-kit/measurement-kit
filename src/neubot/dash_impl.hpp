// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#include "src/common/utils.hpp"
#include <functional>
#include <iostream>
#include <measurement_kit/common.hpp>
#include <measurement_kit/http.hpp>
#include <measurement_kit/neubot.hpp>
#include <stdlib.h>
#include <string>
#include <unistd.h>

#define DASH_MAX_ITERATION 15
#define DASH_SECONDS 2

using namespace mk;
using namespace mk::net;
using namespace mk::http;

namespace mk {
namespace neubot {

static inline void loop_request(Var<Transport> transport,
                                int speed_kbit, Callback<Error> cb,
                                std::string auth,
                                Var<Reactor> reactor, Var<Logger> logger,
                                int iteration = 0) {

    int rate_index = 0;
    Settings settings;
    int DASH_RATES[] = {100,  150,  200,  250,  300,   400,  500,
                                    700,  900,  1200, 1500, 2000,  2500, 3000,
                                    4000, 5000, 6000, 7000, 10000, 20000};
    std::string url = "http://127.0.0.1/dash/download/";

    if (iteration > DASH_MAX_ITERATION) {
        transport->close([=]() { cb(NoError()); });
        return;
    };

    // Bisect
    while (DASH_RATES[rate_index] < speed_kbit) {
        rate_index++;
    }
    rate_index -= 1;
    if (rate_index < 0)
        rate_index = 0;

    int rate_kbit = DASH_RATES[rate_index];
    int count = ((rate_kbit * 1000) / 8) * DASH_SECONDS;
    url += std::to_string(count);
    settings["http/url"] = url;
    double saved_times = mk::time_now();

    http::request_send(
        transport, settings, {
            {"Authorization", auth},
        }, "",
        [=](Error error) {
            if (error) {
                std::cout << "Error: " << (int)error;
                cb(error);
                return;
            }

            request_recv_response(
                transport,
                [=](Error error, Var<Response> res) {
                    if (error) {
                        std::cout << "Error: " << (int)error;
                        cb(error);
                        return;
                    }

                    double new_times = mk::time_now();
                    float length = res->body.length();
                    double time_elapsed = new_times - saved_times;

                    if (time_elapsed < 0) {
                        std::cout << "Error: " << (int)error;
                        cb(error);
                        return;
                    }

                    //TODO

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

                    loop_request(transport, s_k, cb, auth, reactor,
                                 logger, iteration + 1);

                    return;
                },
                reactor, logger);
        });
}

static inline void run_impl(Settings settings, Callback<Error> cb, std::string auth,
                            Var<Reactor> reactor, Var<Logger> logger) {

    request_connect(settings,
                    [=](Error error, Var<Transport> transport) {

                        if (error) {
                            std::cout << "Error: " << (int)error;
                            cb(error);
                            return;
                        }

                        loop_request(transport, 100, cb, auth, reactor, logger);

                        return;
                    },
                    reactor, logger);
}

} // namespace neubot
} // namespace mk
