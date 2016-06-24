// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/ooni/utils_impl.hpp"
#include "src/common/utils.hpp"

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

IPLocation::IPLocation(std::string path_country, std::string path_asn) {
    if (path_asn != "") {
        gi_asn = GeoIP_open(path_asn.c_str(), GEOIP_MEMORY_CACHE);
    }
    if (path_country != "") {
        gi_country = GeoIP_open(path_country.c_str(), GEOIP_MEMORY_CACHE);
    }
}

IPLocation::~IPLocation() {
    if (gi_asn != nullptr) {
        GeoIP_delete(gi_asn);
    }
    if (gi_country != nullptr) {
        GeoIP_delete(gi_country);
    }
}

ErrorOr<std::string> IPLocation::resolve_country_code(std::string ip) {
    if (gi_country == nullptr) {
        return GenericError();
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
        return GenericError();
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

ErrorOr<std::string> IPLocation::resolve_asn(std::string ip) {
    if (gi_asn == nullptr) {
        return GenericError();
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
                    std::string path_asn) {
    IPLocation ip_location(path_country, path_asn);
    ErrorOr<std::string> country_code = ip_location.resolve_country_code(ip);
    if (!country_code) {
        return GenericError();
    }
    ErrorOr<std::string> country_name = ip_location.resolve_country_name(ip);
    if (!country_name) {
        return GenericError();
    }
    ErrorOr<std::string> asn = ip_location.resolve_asn(ip);
    if (!asn) {
        return GenericError();
    }
    json node;
    node["country_code"] = country_code.as_value();
    node["country_name"] = country_name.as_value();
    node["asn"] = asn.as_value();
    return node;
}

} // namespace ooni
} // namespace mk
