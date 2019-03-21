// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_MLABNS_MLABNS_IMPL_HPP
#define SRC_LIBMEASUREMENT_KIT_MLABNS_MLABNS_IMPL_HPP

#include <measurement_kit/common/nlohmann/json.hpp>
#include <measurement_kit/common/version.h>

#include "src/libmeasurement_kit/mlabns/mlabns.hpp"
#include "src/libmeasurement_kit/common/mock.hpp"
#include "src/libmeasurement_kit/http/http.hpp"
#include "src/libmeasurement_kit/regexp/regexp.hpp"

namespace mk {
namespace mlabns {

static inline ErrorOr<std::string> as_query(Settings &settings) {
    std::string query;
    std::string policy = settings.get("mlabns/policy", "");
    std::string country = settings.get("mlabns/country", "");
    std::string metro = settings.get("mlabns/metro", "");
    std::string address_family = settings.get("mlabns/address_family", "");
    if (policy == "" && metro == "" && address_family == "") {
        return {NoError(), query};
    }
    if (policy != "") {
        if (policy != "geo" && policy != "random" && policy != "metro" &&
            policy != "country") {
            return {InvalidPolicyError(), std::string{}};
        }
        if (query != "") {
            query += "&";
        }
        query += "policy=" + policy;
    }
    if (country != "") {
        if (!regexp::valid_country_code(country)) {
            return {InvalidCountryError(), std::string{}};
        }
        if (query != "") {
            query += "&";
        }
        query += "country=" + country;
    }
    if (metro != "") {
        if (!regexp::valid_airport_iata_code(metro)) {
            return {InvalidMetroError(), std::string{}};
        }
        if (query != "") {
            query += "&";
        }
        query += "metro=" + metro;
    }
    if (address_family != "") {
        if (address_family != "ipv4" && address_family != "ipv6") {
            return {InvalidAddressFamilyError(), std::string{}};
        }
        if (query != "") {
            query += "&";
        }
        query += "address_family=" + address_family;
    }
    query = "?" + query;
    return {NoError(), query};
}

static inline http::Headers make_headers(const Settings &settings) noexcept {
  std::string ua;
  if (settings.count("software_name") > 0 &&
      settings.count("software_version") > 0) {
    ua += settings.at("software_name");
    ua += "/";
    ua += settings.at("software_version");
    ua += " ";
  }
  ua += "libmeasurement_kit/" MK_VERSION;
  http::Headers headers;
  http::headers_push_back(headers, "User-Agent", std::move(ua));
  return headers;
}

template <MK_MOCK_AS(http::request_json_no_body, request_json_no_body)>
void query_impl(std::string tool, Callback<Error, Reply> callback,
                Settings settings, SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    ErrorOr<std::string> query = as_query(settings);
    if (!query) {
        callback(query.as_error(), Reply());
        return;
    }
    std::string url = settings.get("mlabns/base_url", std::string{
                                       "https://mlab-ns.appspot.com/"
                                   });
    if (!regexp::lowercase_letters_only(tool)) {
        callback(InvalidToolNameError(), Reply());
        return;
    }
    url += tool;
    url += *query;
    logger->debug("query mlabns for tool %s", tool.c_str());
    logger->debug("mlabns url: %s", url.c_str());
    request_json_no_body("GET", url, make_headers(settings),
        [callback, logger](Error error, SharedPtr<http::Response> /*response*/,
                           nlohmann::json node) {
            if (error) {
                logger->warn("mlabns: HTTP error: %s", error.what());
                callback(error, Reply());
                return;
            }
            Reply reply;
            Error err = NoError();
            try {
                reply.city = node.at("city");
                reply.url = node.at("url");
                for (auto ip2 : node.at("ip")) {
                    reply.ip.push_back(ip2);
                }
                reply.fqdn = node.at("fqdn");
                reply.site = node.at("site");
                reply.country = node.at("country");
            } catch (const std::exception &) {
                err = JsonProcessingError();
            }
            if (err) {
                logger->warn("mlabns: cannot parse json: %s", err.what());
            } else {
                logger->info("Discovered mlab test server: %s",
                             reply.fqdn.c_str());
            }
            callback(err, reply);
        },
        settings, reactor, logger);
}

} // namespace mlabns
} // namespace mk
#endif
