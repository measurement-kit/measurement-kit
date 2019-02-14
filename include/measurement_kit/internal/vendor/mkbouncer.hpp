// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_MKBOUNCER_HPP
#define MEASUREMENT_KIT_MKBOUNCER_HPP

#include <map>
#include <string>
#include <vector>

/// MKBOUNCER_HELPER_WEB_CONNECTIVITY is the Web connectivity helper
#define MKBOUNCER_HELPER_WEB_CONNECTIVITY "web-connectivity"

/// MKBOUNCER_HELPER_TCP_ECHO is the TCP echo helper
#define MKBOUNCER_HELPER_TCP_ECHO "tcp-echo"

/// MKBOUNCER_HELPER_HTTP_RETURN_JSON_HEADERS is an helper that returns
/// a JSON structure containing the received HTTP headers
#define MKBOUNCER_HELPER_HTTP_RETURN_JSON_HEADERS "http-return-json-headers"

namespace mk {
namespace bouncer {

/// Request is a bouncer request
class Request {
 public:
  /// base_url is the bouncer base URL
  std::string base_url = "https://bouncer.ooni.io";

  /// ca_bundle_path is the CA bundle path.
  std::string ca_bundle_path;

  /// helpers contains the list of test helpers to request.
  std::vector<std::string> helpers;

  /// name is the nettest name.
  std::string name;

  /// timeout is the timeout in seconds.
  int64_t timeout = 30;

  /// version is the nettest version.
  std::string version;
};

/// Record is a collector or test-helper record.
class Record {
 public:
  /// type is the record type.
  std::string type;

  /// address is the record address.
  std::string address;

  /// front is the front to use in case of domain fronting.
  std::string front;
};

/// Response is the bouncer response.
class Response {
 public:
  /// good indicates whether we good a good response.
  bool good = false;

  /// collectors lists all available collectors.
  std::vector<Record> collectors;

  /// helpers lists all available test-helpers.
  std::map<std::string, std::vector<Record>> helpers;

  /// logs contains the possibly binary logs.
  std::vector<std::string> logs;
};

/// perform performs @p request and returns a Response.
Response perform(const Request &request) noexcept;

}  // namespace bouncer
}  // namespace mk

// MKBOUNCER_INLINE_IMPL controls whether to include the implementation inline.
#ifdef MKBOUNCER_INLINE_IMPL

#include <sstream>

#include "json.hpp"
#include "mkcurl.hpp"
#include "mkmock.hpp"

#ifdef MKBOUNCER_MOCK
#define MKBOUNCER_HOOK MKMOCK_HOOK_ENABLED
#else
#define MKBOUNCER_HOOK MKMOCK_HOOK_DISABLED
#endif

namespace mk {
namespace bouncer {

// log_body is a helper to log about a body.
static void log_body(const std::string &prefix, const std::string &body,
                     std::vector<std::string> &logs) noexcept {
  std::stringstream ss;
  ss << prefix << " body: " << body;
  logs.push_back(ss.str());
}

Response perform(const Request &request) noexcept {
  Response response;
  curl::Request curl_request;
  curl_request.ca_path = request.ca_bundle_path;
  curl_request.timeout = request.timeout;
  curl_request.method = "POST";
  {
    std::string url = request.base_url;
    url += "/bouncer/net-tests";
    std::swap(url, curl_request.url);
  }
  {
    nlohmann::json doc;
    nlohmann::json nettest;
    nettest["input-hashes"] = nullptr;
    nettest["name"] = request.name;
    nettest["test-helpers"] = request.helpers;
    nettest["version"] = request.version;
    doc["net-tests"].push_back(std::move(nettest));
    std::string body;
    try {
      body = doc.dump();
    } catch (const std::exception &exc) {
      response.logs.push_back(exc.what());
      return response;
    }
    log_body("Request", body, response.logs);
    std::swap(body, curl_request.body);
  }
  curl::Response curl_response = curl::perform(curl_request);
  for (auto &entry : curl_response.logs) {
    response.logs.push_back(std::move(entry.line));
  }
  MKBOUNCER_HOOK(curl_response_error, curl_response.error);
  MKBOUNCER_HOOK(curl_response_status_code, curl_response.status_code);
  if (curl_response.error != 0 || curl_response.status_code != 200) {
    return response;
  }
  MKBOUNCER_HOOK(curl_response_body, curl_response.body);
  {
    log_body("Response", curl_response.body, response.logs);
    try {
      nlohmann::json doc = nlohmann::json::parse(curl_response.body);
      const nlohmann::json &net_tests = doc.at("net-tests")[0];
      if (net_tests.count("collector") > 0) {
        Record record;
        record.address = net_tests.at("collector");
        record.type = "onion";
        response.collectors.push_back(std::move(record));
      }
      if (net_tests.count("collector-alternate") > 0) {
        for (const nlohmann::json &e : net_tests.at("collector-alternate")) {
          Record record;
          record.address = e.at("address");
          record.type = e.at("type");
          if (e.count("front") > 0) {
            record.front = e.at("front");
          }
          response.collectors.push_back(std::move(record));
        }
      }
      if (net_tests.count("test-helpers") > 0) {
        for (auto &e : net_tests.at("test-helpers").items()) {
          Record record;
          record.address = e.value();
          response.helpers[e.key()].push_back(std::move(record));
        }
      }
      if (net_tests.count("test-helpers-alternate") > 0) {
        for (auto &e : net_tests.at("test-helpers-alternate").items()) {
          for (auto &v : e.value()) {
            Record record;
            record.address = v.at("address");
            record.type = v.at("type");
            if (v.count("front") > 0) {
              record.front = v.at("front");
            }
            response.helpers[e.key()].push_back(std::move(record));
          }
        }
      }
    } catch (const std::exception &exc) {
      response.logs.push_back(exc.what());
      return response;
    }
  }
  response.good = true;
  return response;
}

}  // namespace bouncer
}  // namespace mk
#endif  // MKBOUNCER_INLINE_IMPL
#endif  // MEASUREMENT_KIT_MKBOUNCER_HPP
