// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_NEUBOT_DASH_HPP
#define MEASUREMENT_KIT_NEUBOT_DASH_HPP

#include <measurement_kit/common.hpp>
#include <measurement_kit/ext.hpp>
#include <measurement_kit/report.hpp>

namespace mk {
namespace neubot {
namespace dash {

// Run a dash test
void run(Settings settings, Callback<Error,  Var<report::Entry>> cb,
        std::string auth = "",
        Var<Reactor> reactor = Reactor::global(),
        Var<Logger> logger = Logger::global());


} // namespace dash
} // namespace neubot
} // namespace mk

#endif
