// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef SRC_OONI_UTILS_HPP
#define SRC_OONI_UTILS_HPP

#include <measurement_kit/common.hpp>
#include <measurement_kit/http.hpp>
#include <regex>
#include <string>
#include "src/ext/json/src/json.hpp"
#include <GeoIP.h>


using Json = nlohmann::json;


namespace mk {
namespace ooni {

template <decltype(mk::http::get) httpget = mk::http::get>
void ip_lookup(Callback<std::string> callback) {
    httpget("http://geoip.ubuntu.com/lookup",
              [=](Error err, http::Response response) {
                  if (err) {
                      callback(err, "");
                      return;
                  }
                  if (response.status_code != 200) {
                      callback(GenericError(), "");
                      return;
                  }
                  std::smatch m;
                  std::regex regex("<Ip>(.*)</Ip>");
                  if (std::regex_search(response.body, m, regex) == false) {
                      callback(GenericError(), "");
                      return;
                  }
                  callback(NoError(), m[1]);
              }, {}, "", {}, Logger::global(), Poller::global());
}

// this is needed to return a const char instead of a char for asn resolver
inline GEOIP_API const char * MK_GeoIP_name_by_name_gl(GeoIP * gi, const char *host,
                                              GeoIPLookup * gl) {
    return (const char *) GeoIP_name_by_name_gl(gi,host,gl);
}

template<decltype(GeoIP_country_code3_by_name_gl) resolver>
std::string geoip_query(std::string ip, std::string path) {
    GeoIP *gi;
    GeoIPLookup gl;
    gi = GeoIP_open(path.c_str(), GEOIP_MEMORY_CACHE);
    if (gi == nullptr) {
        return "";
    }
    const char *result;
    result = resolver(gi, ip.c_str(), &gl);
    if (result == NULL) {
        return "";
    }
    GeoIP_delete(gi);
    return std::string(result);
}

inline ErrorOr<Json> geoip(std::string ip, std::string path_country, std::string path_asn) {
    Json json;
    std::string result;

    result = geoip_query<GeoIP_country_code3_by_name_gl>(ip, path_country);
    if (result == "") {
        return GenericError();
    }
    json["country_code"]  = result;
    result = geoip_query<GeoIP_country_name_by_name_gl>(ip, path_country);
    if (result == "") {
        return GenericError();
    }
    json["country_name"]  = result;
    
    result = geoip_query<MK_GeoIP_name_by_name_gl>(ip, path_asn);
    if (result == "") {
        return GenericError();
    }
    json["country_asn"]  = result;
    return ErrorOr<Json>(json);
}

} // namespace ooni
} // namespace mk
#endif
