// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_NEUBOT_DASH_HPP
#define MEASUREMENT_KIT_NEUBOT_DASH_HPP

#include <measurement_kit/report.hpp>

namespace mk {
namespace neubot {
namespace dash {

void run(
        std::string measurement_server_hostname,
        std::string auth_token,
        std::string real_address,
        Var<report::Entry> entry,
        Settings settings,
        Reactor reactor,
        Var<Logger> logger,
        Callback<Error> callback
);

void negotiate(
        Var<report::Entry> entry,
        Settings settings,
        Reactor reactor,
        Var<Logger> logger,
        Callback<Error> callback
);

} // namespace dash
} // namespace neubot
} // namespace mk
#endif
