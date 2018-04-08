// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_MLABNS_MLABNS_IMPL_HPP
#define SRC_LIBMEASUREMENT_KIT_MLABNS_MLABNS_IMPL_HPP

#include "src/libmeasurement_kit/mlabns/mlabns.hpp"

#include <measurement_kit/common/json.hpp>
#include "src/libmeasurement_kit/common/mock.hpp"
#include "src/libmeasurement_kit/http/http.hpp"

#include <regex>

namespace mk {
namespace mlabns {

static inline ErrorOr<std::string> as_query(Settings &settings) {
    std::string query;
    std::string policy = settings.get<std::string>("mlabns/policy", "");
    std::string country = settings.get<std::string>("mlabns/country", "");
    std::string metro = settings.get<std::string>("mlabns/metro", "");
    std::string address_family =
        settings.get<std::string>("mlabns/address_family", "");
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
        std::regex valid_country("^[A-Z]{2}$");
        if (!std::regex_match(country, valid_country)) {
            return {InvalidCountryError(), std::string{}};
        }
        if (query != "") {
            query += "&";
        }
        query += "country=" + country;
    }
    if (metro != "") {
        std::regex valid_metro("^[a-z]{3}$");
        if (!std::regex_match(metro, valid_metro)) {
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
    std::regex valid_tool("^[a-z]+$");
    if (!std::regex_match(tool, valid_tool)) {
        callback(InvalidToolNameError(), Reply());
        return;
    }
    url += tool;
    url += *query;
    logger->debug("query mlabns for tool %s", tool.c_str());
    logger->debug("mlabns url: %s", url.c_str());
    request_json_no_body("GET", url, {},
        [callback, logger](Error error, SharedPtr<http::Response> /*response*/,
                           Json json_response) {
            if (error) {
                logger->warn("mlabns: HTTP error: %s", error.what());
                callback(error, Reply());
                return;
            }
            Reply reply;
            Error err = json_process(json_response, [&](auto node) {
                reply.city = node.at("city");
                reply.url = node.at("url");
                for (auto ip2 : node.at("ip")) {
                    reply.ip.push_back(ip2);
                }
                reply.fqdn = node.at("fqdn");
                reply.site = node.at("site");
                reply.country = node.at("country");
            });
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
