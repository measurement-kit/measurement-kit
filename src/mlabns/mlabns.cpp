// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

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

static ErrorOr<std::string> as_query(Settings &settings) {
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
    ErrorOr<std::string> query = as_query(settings);
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
    logger->info("query mlabns for tool %s", tool.c_str());
    logger->debug("mlabns url: %s", url.c_str());
    http::request("GET", url,
                  [callback, logger](Error error, http::Response response) {
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
                          auto node = json::parse(response.body);
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
                      } catch (std::out_of_range &) {
                          callback(JsonParsingError(), Reply());
                          return;
                      }
                      logger->info("mlabns says to use %s", reply.fqdn.c_str());
                      callback(NoError(), reply);
                  },
                  {}, "", settings, logger, reactor);
}

} // namespace mlabns
} // namespace mk
