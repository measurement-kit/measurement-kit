// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "private/neubot/dash_impl.hpp"

namespace mk {
namespace neubot {
namespace dash {

const std::vector<int> &dash_rates() {
    static std::vector<int> DASH_RATES{
        {100,  150,  200,  250,  300,  400,  500,  700,  900,   1200,
         1500, 2000, 2500, 3000, 4000, 5000, 6000, 7000, 10000, 20000,
         30000, 40000, 50000}};
    return DASH_RATES;
}

void run(std::string measurement_server_url, std::string auth_token,
         std::string real_address, Var<report::Entry> entry, Settings settings,
         Var<Reactor> reactor, Var<Logger> logger, Callback<Error> callback) {
    run_impl(measurement_server_url, auth_token, real_address, entry, settings,
             reactor, logger, callback);
}

void negotiate(Var<report::Entry> entry, Settings settings,
               Var<Reactor> reactor, Var<Logger> logger,
               Callback<Error> callback) {
    negotiate_impl(entry, settings, reactor, logger, callback);
}

} // namespace dash
} // namespace neubot
} // namespace mk
