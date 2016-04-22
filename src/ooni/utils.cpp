// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/ooni/utils.hpp"

namespace mk {
namespace ooni {

ErrorOr<Json> geoip(std::string ip, std::string path_country,
                    std::string path_asn) {
    Json json;
    GeoIP *gi;
    GeoIPLookup gl;

    gi = GeoIP_open(path_country.c_str(), GEOIP_MEMORY_CACHE);
    if (gi == nullptr) {
        GeoIP_delete(gi);
        return GenericError();
    }

    const char *result;
    result = GeoIP_country_code3_by_name_gl(gi, ip.c_str(), &gl);
    if (result == nullptr) {
        GeoIP_delete(gi);
        return GenericError();
    }
    json["country_code"] = result;

    result = GeoIP_country_name_by_name_gl(gi, ip.c_str(), &gl);
    if (result == nullptr) {
        GeoIP_delete(gi);
        return GenericError();
    }
    json["country_name"] = result;
    GeoIP_delete(gi);

    gi = GeoIP_open(path_asn.c_str(), GEOIP_MEMORY_CACHE);
    if (gi == nullptr) {
        GeoIP_delete(gi);
        return GenericError();
    }
    char *res;
    res = GeoIP_name_by_name_gl(gi, ip.c_str(), &gl);
    if (res == nullptr) {
        GeoIP_delete(gi);
        return GenericError();
    }
    json["asn"] = res;
    free(res);
    GeoIP_delete(gi);

    return json;
}

} // namespace ooni
} // namespace mk
