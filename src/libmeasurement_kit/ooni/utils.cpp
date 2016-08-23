// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "src/libmeasurement_kit/ooni/utils_impl.hpp"
#include "src/libmeasurement_kit/common/utils.hpp"

namespace mk {
namespace ooni {

void ip_lookup(Callback<Error, std::string> callback, Settings settings,
               Var<Reactor> reactor, Var<Logger> logger) {
    ip_lookup_impl(callback, settings, reactor, logger);
}

void resolver_lookup(Callback<Error, std::string> callback, Settings settings,
                     Var<Reactor> reactor, Var<Logger> logger) {
    resolver_lookup_impl(callback, settings, reactor, logger);
}

IPLocation::IPLocation(std::string path_country_, std::string path_asn_,
                       std::string path_city_) {
    path_asn = path_asn_;
    path_country = path_country_;
    path_city = path_city_;
}

IPLocation::~IPLocation() {
    if (gi_asn != nullptr) {
        GeoIP_delete(gi_asn);
    }
    if (gi_city != nullptr) {
        GeoIP_delete(gi_city);
    }
    if (gi_country != nullptr) {
        GeoIP_delete(gi_country);
    }
}

ErrorOr<std::string> IPLocation::resolve_country_code(std::string ip) {
    if (gi_country == nullptr) {
        gi_country = GeoIP_open(path_country.c_str(), GEOIP_MEMORY_CACHE);
        if (gi_country == nullptr) {
            return CannotOpenGeoIpCountryDatabase();
        }
    }
    GeoIPLookup gl;
    memset(&gl, 0, sizeof(gl));

    const char *result;
    result = GeoIP_country_code_by_name_gl(gi_country, ip.c_str(), &gl);
    if (result == nullptr) {
        return GenericError();
    }
    std::string country_code = result;
    return country_code;

}

ErrorOr<std::string> IPLocation::resolve_country_name(std::string ip) {
    if (gi_country == nullptr) {
        gi_country = GeoIP_open(path_country.c_str(), GEOIP_MEMORY_CACHE);
        if (gi_country == nullptr) {
            return CannotOpenGeoIpCountryDatabase();
        }
    }
    GeoIPLookup gl;
    memset(&gl, 0, sizeof(gl));

    const char *result;
    result = GeoIP_country_name_by_name_gl(gi_country, ip.c_str(), &gl);
    if (result == nullptr) {
        return GenericError();
    }
    std::string country_name = result;
    return country_name;
}

ErrorOr<std::string> IPLocation::resolve_city_name(std::string ip) {
    if (gi_city == nullptr) {
        gi_city = GeoIP_open(path_city.c_str(), GEOIP_MEMORY_CACHE);
        if (gi_country == nullptr) {
            return CannotOpenGeoIpCityDatabase();
        }
    }
    GeoIPRecord *gir = GeoIP_record_by_name(gi_city, ip.c_str());
    if (gir == nullptr) {
        return GenericError();
    }
    std::string result = gir->city;
    GeoIPRecord_delete(gir);
    return result;
}

ErrorOr<std::string> IPLocation::resolve_asn(std::string ip) {
    if (gi_asn == nullptr) {
        gi_asn = GeoIP_open(path_asn.c_str(), GEOIP_MEMORY_CACHE);
        if (gi_asn == nullptr) {
            return CannotOpenGeoIpAsnDatabase();
        }
    }
    GeoIPLookup gl;
    memset(&gl, 0, sizeof(gl));

    char *res;
    res = GeoIP_name_by_name_gl(gi_asn, ip.c_str(), &gl);
    if (res == nullptr) {
        return GenericError();
    }
    std::string asn = res;
    asn = split(asn).front(); // We only want ASXX
    free(res);
    return asn;
}

ErrorOr<json> geoip(std::string ip, std::string path_country,
                    std::string path_asn, std::string path_city) {
    IPLocation ip_location(path_country, path_asn, path_city);
    ErrorOr<std::string> country_code = ip_location.resolve_country_code(ip);
    if (!country_code) {
        return country_code.as_error();
    }
    ErrorOr<std::string> country_name = ip_location.resolve_country_name(ip);
    if (!country_name) {
        return country_name.as_error();
    }
    ErrorOr<std::string> city_name;
    if (path_city != "") {
        city_name = ip_location.resolve_city_name(ip);
        if (!city_name) {
            return city_name.as_error();
        }
    }
    ErrorOr<std::string> asn = ip_location.resolve_asn(ip);
    if (!asn) {
        return asn.as_error();
    }
    json node;
    node["country_code"] = country_code.as_value();
    node["country_name"] = country_name.as_value();
    node["asn"] = asn.as_value();
    if (path_city != "") {
        node["city_name"] = city_name.as_value();
    }
    return node;
}

std::string extract_html_title(std::string body) {
  std::regex TITLE_REGEXP("<title>([\\s\\S]*?)</title>", std::regex::icase);
  std::smatch match;

  if (std::regex_search(body, match, TITLE_REGEXP) && match.size() > 1) {
    return match.str(1);
  }
  return "";
}

bool is_private_ipv4_addr(const std::string &ipv4_addr) {
  std::regex IPV4_PRIV_ADDR(
      "(^127\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3})|"
      "(^192\\.168\\.[0-9]{1,3}\\.[0-9]{1,3})|"
      "(^10\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3})|"
      "(^172\\.1[6-9]\\.[0-9]{1,3}\\.[0-9]{1,3})|"
      "(^172\\.2[0-9]\\.[0-9]{1,3}\\.[0-9]{1,3})|"
      "(^172\\.3[0-1]\\.[0-9]{1,3}\\.[0-9]{1,3})|"
      "localhost"
  );
  std::smatch match;

  if (std::regex_search(ipv4_addr, match, IPV4_PRIV_ADDR) && match.size() > 1) {
    return true;
  }
  return false;
}

bool is_ip_addr(const std::string &ip_addr) {
    struct sockaddr_in sa;
    struct sockaddr_in6 sa6;
    if ((inet_pton(AF_INET, ip_addr.c_str(), &(sa.sin_addr)) == 1) ||
        (inet_pton(AF_INET6, ip_addr.c_str(), &(sa6.sin6_addr)) == 1)) {
        return true;
    }
    return false;
}

} // namespace ooni
} // namespace mk
