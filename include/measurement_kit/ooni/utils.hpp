// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_OONI_UTILS_HPP
#define MEASUREMENT_KIT_OONI_UTILS_HPP

#include <measurement_kit/report.hpp>

namespace mk {
namespace ooni {

void ip_lookup(Callback<Error, std::string> callback, Settings settings = {},
               SharedPtr<Reactor> reactor = Reactor::global(),
               SharedPtr<Logger> logger = Logger::global());

void resolver_lookup(Callback<Error, std::string> callback, Settings = {},
                     SharedPtr<Reactor> reactor = Reactor::global(),
                     SharedPtr<Logger> logger = Logger::global());

report::Entry represent_string(const std::string &s);

// `find_location` returns the client location. Internally, it calls the
// namesake method of orchestrate::Client. This function is here for
// convenience because you may not want to explicitly create a client for
// finding the probe location. In particular, there are cases in which you do
// want to know the probe location that are not related to orchestration.
void find_location(std::string geoip_country_path, std::string geoip_asn_path,
        Settings settings, SharedPtr<Logger> logger,
        Callback<Error &&, std::string &&, std::string &&> &&cb);

} // namespace ooni
} // namespace mk
#endif
