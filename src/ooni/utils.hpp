// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_OONI_UTILS_HPP
#define SRC_OONI_UTILS_HPP

#include <GeoIP.h>
#include <measurement_kit/common.hpp>
#include <measurement_kit/ext.hpp>
#include <measurement_kit/http.hpp>
#include <regex>
#include <string>

using json = nlohmann::json;

namespace mk {
namespace ooni {

void ip_lookup(Callback<Error, std::string> callback, Settings settings = {},
               Var<Reactor> reactor = Reactor::global(),
               Var<Logger> logger = Logger::global());

ErrorOr<json> geoip(std::string ip, std::string path_country,
                    std::string path_asn);

} // namespace ooni
} // namespace mk
#endif
