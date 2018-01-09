// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/ooni/resources_impl.hpp"

namespace mk {
namespace ooni {
namespace resources {

void get_latest_release(Callback<Error, std::string> cb, Settings settings,
                        SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    get_latest_release_impl(cb, settings, reactor, logger);
}

void get_manifest_as_json(
        std::string latest, Callback<Error, Json> cb,
        Settings settings, SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    get_manifest_as_json_impl(latest, cb, settings, reactor, logger);
}

void get_resources_for_country(std::string version,
                               Json manifest,
                               std::string country,
                               Callback<Error> callback,
                               Settings settings,
                               SharedPtr<Reactor> reactor,
                               SharedPtr<Logger> logger) {
    get_resources_for_country_impl(version, manifest, country, callback,
                                   settings, reactor, logger);
}

void get_resources(std::string latest, std::string country,
                   Callback<Error> callback, Settings settings,
                   SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    get_resources_impl(latest, country, callback, settings, reactor, logger);
}

} // namespace resources
} // namespace mk
} // namespace ooni
