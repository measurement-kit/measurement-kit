// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/neubot/negotiation_impl.hpp"

namespace mk {
namespace neubot {

void run_negotiation(Settings settings, Callback<Error> cb,
            Var<Reactor> reactor, Var<Logger> logger) {
    run_negotiation_impl(settings, cb, reactor, logger);
}

} // namespace neubot
} // namespace mk
