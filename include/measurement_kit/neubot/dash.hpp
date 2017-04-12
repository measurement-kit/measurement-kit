// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_NEUBOT_DASH_HPP
#define MEASUREMENT_KIT_NEUBOT_DASH_HPP

#include <measurement_kit/report.hpp>

namespace mk {
namespace neubot {
namespace dash {

void run_with(
        std::string measurement_server_url,
        std::string auth_token,
        Settings settings,
        Var<Reactor> reactor,
        Var<Logger> logger,
        Callback<
                Error /*error*/,
                Var<report::Entry> /*entry*/
        > callback
);

void negotiate_with(
        std::string negotiate_server_url,
        Settings settings,
        Var<Reactor> reactor,
        Var<Logger> logger,
        Callback<
                Error /*error*/,
                Var<report::Entry> /*entry*/
        > callback
);

void negotiate(
        Settings settings,
        Var<Reactor> reactor,
        Var<Logger> logger,
        Callback<
                Error /*error*/,
                Var<report::Entry> /*entry*/
        > callback
);

} // namespace dash
} // namespace neubot
} // namespace mk
#endif
