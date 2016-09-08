// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "resources_impl.hpp"

namespace mk {
namespace ooni {
namespace resources {

void get_latest_release_url(Callback<Error, std::string> cb, Settings settings,
                            Var<Reactor> reactor, Var<Logger> logger) {
    get_latest_release_url_impl(cb, settings, reactor, logger);
}

} // namespace resources
} // namespace mk
} // namespace ooni
