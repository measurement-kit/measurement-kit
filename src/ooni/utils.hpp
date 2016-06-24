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

void resolver_lookup(Callback<Error, std::string> callback, Settings settings = {},
                     Var<Reactor> reactor = Reactor::global(),
                     Var<Logger> logger = Logger::global());

class IPLocation {
    public:
      IPLocation(std::string path_country, std::string path_asn);

      ~IPLocation();

      ErrorOr<std::string> resolve_country_code(std::string ip);

      ErrorOr<std::string> resolve_country_name(std::string ip);

      ErrorOr<std::string> resolve_asn(std::string ip);

    private:
      GeoIP *gi_asn = nullptr;
      GeoIP *gi_country = nullptr;
      Var<Logger> logger = Logger::make();

};

ErrorOr<json> geoip(std::string ip, std::string path_country,
                    std::string path_asn);

} // namespace ooni
} // namespace mk
#endif
