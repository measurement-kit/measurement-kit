// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef PRIVATE_MLABNS_MLABNS_IMPL_HPP
#define PRIVATE_MLABNS_MLABNS_IMPL_HPP

#include <measurement_kit/common/json.hpp>
#include "private/common/mock.hpp"

#include <measurement_kit/ext.hpp>
#include <measurement_kit/http.hpp>
#include <measurement_kit/mlabns.hpp>
#include <regex>

namespace mk {
namespace mlabns {

static inline ErrorOr<std::string> as_query(Settings &settings) {
    std::string query;
    std::string policy = settings.get<std::string>("mlabns/policy", "");
    std::string metro = settings.get<std::string>("mlabns/metro", "");
    std::string address_family =
        settings.get<std::string>("mlabns/address_family", "");
    if (policy == "" && metro == "" && address_family == "") {
        return query;
    }
    if (policy != "") {
        if (policy != "geo" && policy != "random" && policy != "metro" &&
            policy != "country") {
            return InvalidPolicyError();
        }
        if (query != "") {
            query += "&";
        }
        query += "policy=" + policy;
    }
    if (metro != "") {
        std::regex valid_metro("^[a-z]{3}$");
        if (!std::regex_match(metro, valid_metro)) {
            return InvalidMetroError();
        }
        if (query != "") {
            query += "&";
        }
        query += "metro=" + metro;
    }
    if (address_family != "") {
        if (address_family != "ipv4" && address_family != "ipv6") {
            return InvalidAddressFamilyError();
        }
        if (query != "") {
            query += "&";
        }
        query += "address_family=" + address_family;
    }
    query = "?" + query;
    return query;
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
