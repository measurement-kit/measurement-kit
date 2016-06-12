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

static Error geoip_resolve_country (std::string ip, std::string path_country, json &node) { 
    GeoIP *gi;
    GeoIPLookup gl;
    memset (&gl, 0, sizeof(gl));
    
    gi = GeoIP_open(path_country.c_str(), GEOIP_MEMORY_CACHE);
    if (gi == nullptr) {
        return GenericError();
    }

    const char *result;
    result = GeoIP_country_code_by_name_gl(gi, ip.c_str(), &gl);
    if (result == nullptr) {
        GeoIP_delete(gi);
        return GenericError();
    }
    node["country_code"] = result;

    result = GeoIP_country_name_by_name_gl(gi, ip.c_str(), &gl);
    if (result == nullptr) {
        GeoIP_delete(gi);
        return GenericError();
    }
    node["country_name"] = result;
    GeoIP_delete(gi);
    return NoError();
}

static Error geoip_resolve_asn (std::string ip, std::string path_asn, json &node) { 
    GeoIP *gi;
    GeoIPLookup gl;
    memset (&gl, 0, sizeof(gl));
    
    gi = GeoIP_open(path_asn.c_str(), GEOIP_MEMORY_CACHE);
    if (gi == nullptr) {
        return GenericError();
    }
    char *res;
    res = GeoIP_name_by_name_gl(gi, ip.c_str(), &gl);
    if (res == nullptr) {
        GeoIP_delete(gi);
        return GenericError();
    }
    node["asn"] = res;
    node["asn"] = split(node["asn"]).front(); // We only want ASXXXX
    free(res);
    GeoIP_delete(gi);
    return NoError();
}

ErrorOr<json> geoip(std::string ip, std::string path_country,
                    std::string path_asn) {
    json node{
        // Set sane values such that we should not have errors accessing
        // output even when lookup may fail for current database
        {"country_code", "ZZ"},
        {"country_name", "ZZ"},
        {"asn", "AS0"},
    };
    Error err = geoip_resolve_country (ip, path_country, node);
    if (err) {
        return err;
    }
    err = geoip_resolve_asn (ip, path_asn, node);
    if (err) {
        return err;
    }
    return node;
}

} // namespace ooni
} // namespace mk
