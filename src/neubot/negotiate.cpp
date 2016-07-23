// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/neubot/negotiate_impl.hpp"

namespace mk {
namespace neubot {
namespace negotiate {

void run(Callback<Error> cb, Settings settings, Var<Reactor> reactor,
         Var<Logger> logger) {
    run_impl(cb, settings, reactor, logger);
}

} // namespace negotiate
} // namespace neubot
} // namespace mk
