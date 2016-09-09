// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_OONI_UTILS_HPP
#define SRC_LIBMEASUREMENT_KIT_OONI_UTILS_HPP

#include <measurement_kit/ooni.hpp>

#include <GeoIP.h>
#include <GeoIPCity.h>

using json = nlohmann::json;

namespace mk {
namespace ooni {

void ip_lookup(Callback<Error, std::string> callback, Settings settings = {},
               Var<Reactor> reactor = Reactor::global(),
               Var<Logger> logger = Logger::global());

void resolver_lookup(Callback<Error, std::string> callback, Settings = {},
                     Var<Reactor> reactor = Reactor::global(),
                     Var<Logger> logger = Logger::global());

class IPLocation {
  public:
    IPLocation(std::string path_country, std::string path_asn,
               std::string path_city = "");

    ~IPLocation();

    ErrorOr<std::string>
    resolve_country_code(std::string ip, Var<Logger> = Logger::global());

    ErrorOr<std::string>
    resolve_country_name(std::string ip, Var<Logger> = Logger::global());

    ErrorOr<std::string>
    resolve_city_name(std::string ip, Var<Logger> = Logger::global());

    ErrorOr<std::string>
    resolve_asn(std::string ip, Var<Logger> = Logger::global());

  private:
    std::string path_country = "";
    std::string path_asn = "";
    std::string path_city = "";
    GeoIP *gi_asn = nullptr;
    GeoIP *gi_country = nullptr;
    GeoIP *gi_city = nullptr;
};

ErrorOr<json> geoip(std::string ip, std::string path_country,
                    std::string path_asn, std::string path_city = "");

std::string extract_html_title(std::string body);

bool is_private_ipv4_addr(const std::string &ipv4_addr);

// Returns true if it's a ipv4 or ipv6 ip address
bool is_ip_addr(const std::string &ip_addr);

} // namespace ooni
} // namespace mk
#endif
