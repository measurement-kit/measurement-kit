// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/ooni/utils.hpp"

namespace mk {
namespace ooni {

ErrorOr<json> geoip(std::string ip, std::string path_country,
                    std::string path_asn) {
    json node;
    GeoIP *gi;
    GeoIPLookup gl;
    memset (&gl, 0, sizeof(gl));
    
    gi = GeoIP_open(path_country.c_str(), GEOIP_MEMORY_CACHE);
    if (gi == nullptr) {
        return GenericError();
    }

    const char *result;
    result = GeoIP_country_code3_by_name_gl(gi, ip.c_str(), &gl);
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
    free(res);
    GeoIP_delete(gi);

    return node;
}

} // namespace ooni
} // namespace mk
