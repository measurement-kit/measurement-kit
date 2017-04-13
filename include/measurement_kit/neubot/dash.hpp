// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_NEUBOT_DASH_HPP
#define MEASUREMENT_KIT_NEUBOT_DASH_HPP

#include <measurement_kit/report.hpp>

namespace mk {
namespace neubot {
namespace dash {

void run(
        std::string measurement_server_url,
        std::string auth_token,
        Var<report::Entry> entry,
        Settings settings,
        Var<Reactor> reactor,
        Var<Logger> logger,
        Callback<Error> callback
);

void negotiate(
        Var<report::Entry> entry,
        Settings settings,
        Var<Reactor> reactor,
        Var<Logger> logger,
        Callback<Error> callback
);

} // namespace dash
} // namespace neubot
} // namespace mk
#endif
