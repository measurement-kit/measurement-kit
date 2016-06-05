// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_NEUBOT_NEGOTIATE_HPP
#define MEASUREMENT_KIT_NEUBOT_NEGOTIATE_HPP

#include <measurement_kit/common.hpp>

namespace mk {
namespace neubot {
namespace negotiate {

void run_negotiation(Settings settings, Callback<Error> cb,
        Var<Reactor> reactor = Reactor::global(),
        Var<Logger> logger = Logger::global());

} // namespace negotiate
} // namespace neubot
} // namespace mk

#endif
