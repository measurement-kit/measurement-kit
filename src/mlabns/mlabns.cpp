// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <functional>
#include <initializer_list>
#include <map>
#include <measurement_kit/common.hpp>
#include <measurement_kit/http.hpp>
#include <measurement_kit/mlabns.hpp>
#include <regex>
#include <string>
#include <utility>
#include <vector>
#include "src/ext/json/src/json.hpp"

using json = nlohmann::json;

namespace mk {
namespace mlabns {

Query::Query(std::initializer_list<std::pair<std::string, std::string>> in) {
    for (auto &s : in) {
        if (s.first == "policy")
            policy = s.second;
        else if (s.first == "metro")
            metro = s.second;
        else if (s.first == "address_family")
            address_family = s.second;
        else {
            /* nothing */
        }
    }
}

ErrorOr<std::string> Query::as_query() {
    std::string query;
    if (policy == "" && metro == "" && address_family == "") {
        return query;
    }
    if (policy != "") {
        if (policy != "geo" && policy != "random" && policy != "metro" &&
            policy != "country") {
            return InvalidPolicyError();
        }
        if (query != "") query += "&";
        query += "policy=" + policy;
    }
    if (metro != "") {
        std::regex valid_metro("^[a-z]{3}$");
        if (!std::regex_match(metro, valid_metro)) {
            return InvalidMetroError();
        }
        if (query != "") query += "&";
        query += "metro=" + metro;
    }
    if (address_family != "") {
        if (address_family != "ipv4" && address_family != "ipv6") {
            return InvalidAddressFamilyError();
        }
        if (query != "") query += "&";
        query += "address_family=" + address_family;
    }
    query = "?" + query;
    return query;
}

void query(std::string tool, Callback<Reply> callback,
           Query request) {
    ErrorOr<std::string> query = request.as_query();
    if (!query) {
        callback(query.as_error(), Reply());
        return;
    }
    std::string url = "https://mlab-ns.appspot.com/";
    std::regex valid_tool("^[a-z]+$");
    if (!std::regex_match(tool, valid_tool)) {
        callback(InvalidToolNameError(), Reply());
        return;
    }
    url += tool;
    url += *query;
    mk::debug("about to call the request function");
    http::request(
        {
            {"method", "GET"}, {"url", url},
        },
        [callback](Error error, http::Response response) {
            if (error) {
                callback(error, Reply());
                return;
            }
            if (response.status_code != 200) {
                callback(UnexpectedHttpStatusCodeError(), Reply());
                return;
            }

            Reply reply;
            try {
                auto node = json::parse (response.body);
                reply.city = node["city"];
                reply.url = node["url"];
                for (auto ip2 : node["ip"]) {
                    reply.ip.push_back(ip2);
                }
                reply.fqdn = node["fqdn"];
                reply.site = node["site"];
                reply.country = node["country"];
            } catch (std::invalid_argument &) {
                callback(JsonParsingError(), Reply());
                return;
            } catch (std::out_of_range &){
                callback(JsonParsingError(), Reply());
                return;
            }
            callback(NoError(), reply);
        });
}

} // namespace mlabns
} // namespace mk
