// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_MKCOLLECTOR_HPP
#define MEASUREMENT_KIT_MKCOLLECTOR_HPP

#include <stdint.h>

#include <string>
#include <vector>

namespace mk {
namespace collector {

/// Settings contains common network related settings.
class Settings {
 public:
  /// base_url is the OONI collector base_url
  std::string base_url;

  /// ca_bundle_path is the path to the CA bundle (required on mobile)
  std::string ca_bundle_path;

  /// timeout is the whole operation timeout (in seconds)
  int64_t timeout = 30;
};

/// LoadResult is the result of loading a structure from JSON.
template <typename Type>
class LoadResult {
 public:
  /// good indicates whether loading succeeded.
  bool good = false;

  /// reason indicates the failure reason on failure.
  std::string reason;

  /// value is the parsed value on success.
  Type value = {};
};

/// OpenRequest is a request to open a report with a collector.
class OpenRequest {
 public:
  /// probe_asn is the probe ASN
  std::string probe_asn;

  /// probe_cc is the probe country code
  std::string probe_cc;

  /// software_name is the name of the application
  std::string software_name;

  /// software_version is the version of the application
  std::string software_version;

  /// test_name is the nettest name
  std::string test_name;

  /// test_start_time is the time when the test started
  std::string test_start_time;

  /// test_version is the nettest version
  std::string test_version;
};

/// open_request_from_measurement initializes an OpenRequest structure
/// from an existing @p measurement. This is the function that you want
/// to call when you want to resubmit a specific measurement.
LoadResult<OpenRequest> open_request_from_measurement(
    const std::string &measurement) noexcept;

/// OpenResponse is the response to an open request.
struct OpenResponse {
  /// good indicates whether we succeded.
  bool good = false;

  /// report_id is the report ID (only meaningful on success).
  std::string report_id;

  /// logs contains the logs.
  std::vector<std::string> logs;
};

/// open opens a report with a collector.
OpenResponse open(const OpenRequest &request,
                  const Settings &settings) noexcept;

/// UpdateRequest is a request to update a report with a new measurement.
struct UpdateRequest {
  /// report_id is the report ID.
  std::string report_id;

  /// content is the measurement entry serialised as a string.
  std::string content;
};

/// UpdateResponse is a response to an update request.
struct UpdateResponse {
  bool good = false;

  /// logs contains the logs.
  std::vector<std::string> logs;
};

/// update updates a report by adding a new measurement.
UpdateResponse update(const UpdateRequest &request,
                      const Settings &settings) noexcept;

/// CloseRequest is a request to close a report.
struct CloseRequest {
  /// report_id is the report ID
  std::string report_id;
};

/// CloseResponse is a response to a close request
struct CloseResponse {
  /// good indicates whether the operation succeeded
  bool good = false;

  /// logs contains the logs.
  std::vector<std::string> logs;
};

/// close closes a report.
CloseResponse close(const CloseRequest &request,
                    const Settings &settings) noexcept;

}  // namespace collector
}  // namespace mk

// The implementation can be included inline by defining this preprocessor
// symbol. If you only care about API, you can stop reading here.
#ifdef MKCOLLECTOR_INLINE_IMPL

#include <stdexcept>
#include <sstream>

#include "json.hpp"
#include "mkcurl.hpp"
#include "mkmock.hpp"

#ifdef MKCOLLECTOR_MOCK
#define MKCOLLECTOR_HOOK MKMOCK_HOOK_ENABLED
#else
#define MKCOLLECTOR_HOOK MKMOCK_HOOK_DISABLED
#endif

namespace mk {
namespace collector {

// log_body is a helper to log about a body.
static void log_body(const std::string &prefix, const std::string &body,
                     std::vector<std::string> &logs) noexcept {
  std::stringstream ss;
  ss << prefix << " body: " << body;
  logs.push_back(ss.str());
}

LoadResult<OpenRequest> open_request_from_measurement(
    const std::string &measurement) noexcept {
  LoadResult<OpenRequest> result;
  nlohmann::json doc;
  try {
    doc = nlohmann::json::parse(measurement);
    doc.at("probe_asn").get_to(result.value.probe_asn);
    doc.at("probe_cc").get_to(result.value.probe_cc);
    doc.at("software_name").get_to(result.value.software_name);
    doc.at("software_version").get_to(result.value.software_version);
    doc.at("test_name").get_to(result.value.test_name);
    doc.at("test_start_time").get_to(result.value.test_start_time);
    doc.at("test_version").get_to(result.value.test_version);
  } catch (const std::exception &exc) {
    result.reason = exc.what();
    return result;
  }
  result.good = true;
  return result;
}

OpenResponse open(const OpenRequest &request,
                  const Settings &settings) noexcept {
  OpenResponse response;
  curl::Request curl_request;
  curl_request.ca_path = settings.ca_bundle_path;
  curl_request.timeout = settings.timeout;
  curl_request.method = "POST";
  curl_request.headers.push_back("Content-Type: application/json");
  {
    std::string url = settings.base_url;
    url += "/report";
    std::swap(url, curl_request.url);
  }
  {
    std::string body;
    nlohmann::json doc;
    doc["data_format_version"] = "0.2.0";
    doc["format"] = "json";
    doc["input_hashes"] = nlohmann::json::array();
    doc["probe_asn"] = request.probe_asn;
    doc["probe_cc"] = request.probe_cc;
    doc["software_name"] = request.software_name;
    doc["software_version"] = request.software_version;
    doc["test_name"] = request.test_name;
    doc["test_start_time"] = request.test_start_time,
    doc["test_version"] = request.test_version;
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
  MKCOLLECTOR_HOOK(open_response_error, curl_response.error);
  MKCOLLECTOR_HOOK(open_response_status_code, curl_response.status_code);
  if (curl_response.error != 0 || curl_response.status_code != 200) {
    return response;
  }
  MKCOLLECTOR_HOOK(open_response_body, curl_response.body);
  {
    log_body("Response", curl_response.body, response.logs);
    nlohmann::json doc;
    try {
      doc = nlohmann::json::parse(curl_response.body);
      doc.at("report_id").get_to(response.report_id);
    } catch (const std::exception &exc) {
      response.logs.push_back(exc.what());
      return response;
    }
  }
  response.good = true;
  return response;
}

UpdateResponse update(const UpdateRequest &request,
                      const Settings &settings) noexcept {
  UpdateResponse response;
  curl::Request curl_request;
  curl_request.ca_path = settings.ca_bundle_path;
  curl_request.timeout = settings.timeout;
  curl_request.method = "POST";
  curl_request.headers.push_back("Content-Type: application/json");
  {
    std::string url = settings.base_url;
    url += "/report/";
    url += request.report_id;
    std::swap(url, curl_request.url);
  }
  {
    std::string body;
    nlohmann::json doc;
    doc["format"] = "json";
    try {
      auto content = nlohmann::json::parse(request.content);
      // Implementation note: the following checks rely on the fact that
      // we're inside a try...catch block and nlohmann/json will throw if
      // content is not an object, a field is missing, etc. That's also
      // why we're using throw to leave this block rather than return.
      if (content.at("data_format_version") != "0.2.0") {
        throw std::runtime_error("Unsupported data_format_version");
      }
      if (content.at("report_id") != request.report_id) {
        throw std::runtime_error("The report_id is inconsistent");
      }
      doc["content"] = std::move(content);
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
  MKCOLLECTOR_HOOK(update_response_error, curl_response.error);
  MKCOLLECTOR_HOOK(update_response_status_code, curl_response.status_code);
  if (curl_response.error != 0 || curl_response.status_code != 200) {
    return response;
  }
  log_body("Response", curl_response.body, response.logs);
  response.good = true;
  return response;
}

CloseResponse close(const CloseRequest &request,
                    const Settings &settings) noexcept {
  CloseResponse response;
  curl::Request curl_request;
  curl_request.method = "POST";
  curl_request.ca_path = settings.ca_bundle_path;
  curl_request.timeout = settings.timeout;
  {
    std::string url = settings.base_url;
    url += "/report/";
    url += request.report_id;
    url += "/close";
    std::swap(url, curl_request.url);
  }
  curl::Response curl_response = curl::perform(curl_request);
  for (auto &entry : curl_response.logs) {
    response.logs.push_back(std::move(entry.line));
  }
  if (curl_response.error != 0 || curl_response.status_code != 200) {
    return response;
  }
  log_body("Response", curl_response.body, response.logs);
  response.good = true;
  return response;
}

}  // namespace collector
}  // namespace mk
#endif  // MKCOLLECTOR_INLINE_IMPL
#endif  // MEASUREMENT_KIT_MKCOLLECTOR_HPP
