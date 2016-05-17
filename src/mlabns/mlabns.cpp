// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/mlabns/mlabns.hpp"
#include "src/ext/json/src/json.hpp"
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

using json = nlohmann::json;

namespace mk {
namespace mlabns {

ErrorOr<std::string> as_query(Settings &settings) {
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

void query(std::string tool, Callback<Error, Reply> callback, Settings settings,
           Var<Reactor> reactor, Var<Logger> logger) {
    query_debug(tool, callback, settings, reactor, logger);
}

} // namespace mlabns
} // namespace mk
