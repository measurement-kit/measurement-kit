// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/neubot/negotiate_impl.hpp"

namespace mk {
namespace neubot {
namespace negotiate {

void run_negotiation(Settings settings, Callback<Error> cb,
                     Var<Reactor> reactor, Var<Logger> logger) {
    run_impl(settings, cb, reactor, logger);
}

} // namespace negotiate
} // namespace neubot
} // namespace mk
