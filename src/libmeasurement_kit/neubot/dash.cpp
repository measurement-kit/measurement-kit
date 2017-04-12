// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../neubot/dash_impl.hpp"

namespace mk {
namespace neubot {
namespace dash {

void run_with(std::string measurement_server_url, std::string auth_token,
              Settings settings, Var<Reactor> reactor, Var<Logger> logger,
              Callback<Error, Var<report::Entry>> callback) {
    run_with_impl(measurement_server_url, auth_token, settings, reactor,
                  logger, callback);
}

void negotiate(Var<report::Entry> entry, Settings settings,
               Var<Reactor> reactor, Var<Logger> logger,
               Callback<Error> callback) {
    negotiate_impl(entry, settings, reactor, logger, callback);
}

} // namespace dash
} // namespace neubot
} // namespace mk
