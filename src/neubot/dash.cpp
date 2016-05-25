// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/neubot/dash_impl.hpp"

namespace mk {
namespace neubot {

void run(Settings settings, Callback<Error> cb, std::string auth,
            Var<Reactor> reactor, Var<Logger> logger) {
    run_impl(settings, cb, auth, reactor, logger);
}

} // namespace neubot
} // namespace mk
