// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/ooni/utils.hpp"

namespace mk {
namespace ooni {

ErrorOr<Json> geoip(std::string ip, std::string path_country,
                           std::string path_asn) {
    Json json;
    std::string result;

    result = geoip_query<GeoIP_country_code3_by_name_gl>(ip,  path_country);
    if (result == "") {
        return GenericError();
    }
    json["country_code"] = result;
    result = geoip_query<GeoIP_country_name_by_name_gl>(ip,   path_country);
    if (result == "") {
        return GenericError();
    }
    json["country_name"] = result;

    result = geoip_query<MK_GeoIP_name_by_name_gl>(ip,        path_asn);
    if (result == "") {
        return GenericError();
    }
    json["asn"] = result;
    return json;
}

} //namespace ooni
} //namespace mk
