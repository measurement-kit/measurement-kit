// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "resources_impl.hpp"

namespace mk {
namespace ooni {
namespace resources {

void get_latest_release(Callback<Error, std::string> cb, Settings settings,
                        Var<Reactor> reactor, Var<Logger> logger) {
    get_latest_release_impl(cb, settings, reactor, logger);
}

void get_manifest_as_json(
        std::string latest, Callback<Error, nlohmann::json> cb,
        Settings settings, Var<Reactor> reactor, Var<Logger> logger) {
    get_manifest_as_json_impl(latest, cb, settings, reactor, logger);
}

} // namespace resources
} // namespace mk
} // namespace ooni
