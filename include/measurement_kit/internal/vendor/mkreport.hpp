// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_MKREPORT_HPP
#define MEASUREMENT_KIT_MKREPORT_HPP

#include <stdint.h>

#include <chrono>
#include <map>
#include <string>
#include <vector>

namespace mk {
namespace report {

/// ooni_date_now formats the current date and time according to the
/// format that is expected by the OONI collector.
std::string ooni_date_now() noexcept;

/// monotonic_seconds_now returns the current time in seconds according
/// to the C++11 monotonic clock.
double monotonic_seconds_now() noexcept;

/// Measurement contains info about a measurement. The code that will
/// actually run a measurement is not here. You should (1) save the
/// measurement input (an empty string is fine), (2) call start() before
/// starting the measurements, (3) run the measurement and store its
/// JSON result in test_keys, and finally (4) call stop(). You can then
/// pass the Measurement object to Report for submission.
class Measurement {
 public:
  /// input is the measurement input.
  std::string input;

  /// start_time is the time when the measurement started.
  std::string start_time;

  /// test_keys contains the measurement test keys as a serialized
  /// JSON object. Measurement specific code will produce this.
  std::string test_keys;

  /// runtime is the time for which the measurement run.
  double runtime = 0.0;

  /// start records the measurement start_time and the moment in
  /// which the test started. That will be useful later to compute
  /// the test runtime. The moment in which the test started is
  /// computed using a monotonic clock.
  void start() noexcept;

  /// stop computes the test_runtime variable. The delta time
  /// is computed using a monotonic clock.
  void stop() noexcept;

 private:
  double beginning_ = 0.0;
};

/// Report represent a report to be submitted to the OONI collector.
class Report {
 public:
  /// annotation contains optional results annotations.
  std::map<std::string, std::string> annotations;

  /// probe_asn is the probe ASN.
  std::string probe_asn;

  /// probe_cc is the probe country code.
  std::string probe_cc;

  /// software_name is the name of the application.
  std::string software_name;

  /// software_version is the version of the application.
  std::string software_version;

  /// test_name is the nettest name.
  std::string test_name;

  /// test_version is the nettest version.
  std::string test_version;

  /// test_start_time is the time when the test started.
  std::string test_start_time;

  /// collector_base_url is the OONI collector base URL.
  std::string collector_base_url;

  /// ca_bundle_path is the path to the CA bundle (required on mobile).
  std::string ca_bundle_path;

  /// id is the identifier of the report.
  std::string id;

  /// timeout is the timeout for HTTP requests (in seconds).
  int64_t timeout = 30;

  /// Report creates a new empty report.
  Report() noexcept;

  /// The copy constructor will copy the internal state variables.
  Report(const Report &) = default;

  /// The copy assignment will copy the internal state variables.
  Report &operator=(const Report &) = default;

  /// The move constructor will move the internal state variables.
  Report(Report &&) noexcept = default;

  /// The move assignment will move the internal state variables.
  Report &operator=(Report &&) noexcept = delete;

  /// autodiscover_collector is like autodiscover_collector_with_bouncer
  /// except that it uses the default bouncer_base_url.
  bool autodiscover_collector(std::vector<std::string> &logs) noexcept;

  /// autodiscover_collector_with_bouncer will autodiscover and configure the
  /// collector_base_url to the first collector returned to the bouncer by
  /// using the @p bouncer_base_url as boucer URL. Returns true on success and
  /// false on failure. In the latter case, please check the @p logs. Note that
  /// if @p bouncer_base_url is empty, this function will use the default URL
  /// for the bouncer, thereby being equivalent to autodiscover_collector.
  bool autodiscover_collector_with_bouncer(
      const std::string &bouncer_base_url,
      std::vector<std::string> &logs) noexcept;

  /// autodiscover_probe_asn_probe_cc will autodiscover and configure the
  /// probe_asn and probe_cc settings. Returns true on success and false
  /// on failure. Please, check the @p logs in the latter case.
  bool autodiscover_probe_asn_probe_cc(
      const std::string &asn_db_path, const std::string &country_db_path,
      std::vector<std::string> &logs) noexcept;

  /// open opens a report with the configured OONI collector. This function
  /// will return true on success and false on failure. In case of failure, you
  /// SHOULD inspect the @p logs, which will explain what went wrong. The
  /// algorithm is the following: open will fail if the report ID is nonempty
  /// as well as if some mandatory field is missing; otherwise it will attempt
  /// to open the report, write the report ID, and return whether it succeded
  /// in opening the report or not.
  bool open(std::vector<std::string> &logs) noexcept;

  /// submit_measurement turns @p measurement into a JSON object and submits it
  /// to the configured OONI collector. This function will return true on
  /// success and false on failure. This function will also fail immediately if
  /// the report ID is empty or if the collector base URL is empty. In case of
  /// failure, inspect the @p logs to have a sense of what went wrong.
  bool submit_measurement(
      Measurement measurement, std::vector<std::string> &logs) noexcept;

  /// submit_measurement_json is like submit_measurement except that we have
  /// in input an already well formed measurement as JSON.
  bool submit_measurement_json(
      std::string measurement, std::vector<std::string> &logs) noexcept;

  /// close closes the report with the OONI collector. This function will
  /// return true on success and false on failure. In case of failure, you
  /// SHOULD inspect the @p logs, which will explain what went wrong. The
  /// algorithm is the following: if there is no report ID, then this
  /// function will return false; otherwise, if there is no collector base
  /// URL, this function will return false; otherwise, it will attempt to
  /// close the report, clear the report ID, and return whether it succeded
  /// in closing the report or not.
  bool close(std::vector<std::string> &logs) noexcept;

  /// ~Report cleans the allocated resources. Notably, if a report is open
  /// and it has not been closed, this function will also close it.
  ~Report() noexcept;

  /// make_content crates a serialized JSON measurement object in @p content
  /// from the @p measurement. This function will return true on success, and
  /// false on failure. In the latter case, the @p logs parameter will
  /// contain explanatory logs.
  bool make_content(const Measurement &measurement, std::string &content,
                    std::vector<std::string> &logs) noexcept;
};

/// resubmit_measurement resubmits the @p serialized_json measurement using
/// the discovered collector. The @p ca_bundle_path and @p timeout arguments
/// have the usual nonsurprising meaning. This function will submit the report
/// overwriting any previous report ID. Returns a boolean indicating whether
/// we succeded. Check the @p logs on failure. Check the value of @p id to
/// know what is the new report ID for this measurement.
bool resubmit_measurement(
    std::string serialized_json, std::string ca_bundle_path,
    int64_t timeout, std::vector<std::string> &logs, std::string &id) noexcept;

}  // namespace report
}  // namespace mk

// The implementation can be included inline by defining this preprocessor
// symbol. If you only care about API, you can stop reading here.
#ifdef MKREPORT_INLINE_IMPL

#include <exception>
#include <utility>

#include "date.h"
#include "json.hpp"
#include "mkbouncer.hpp"
#include "mkcollector.hpp"
#include "mkiplookup.hpp"
#include "mkmock.hpp"
#include "mkmmdb.hpp"

// MKREPORT_MOCK controls whether to enable mocking
#ifdef MKREPORT_MOCK
#define MKREPORT_HOOK MKMOCK_HOOK_ENABLED
#define MKREPORT_HOOK_ALLOC MKMOCK_HOOK_ALLOC_ENABLED
#else
#define MKREPORT_HOOK MKMOCK_HOOK_DISABLED
#define MKREPORT_HOOK_ALLOC MKMOCK_HOOK_ALLOC_DISABLED
#endif

namespace mk {
namespace report {

std::string ooni_date_now() noexcept {
  // Implementation note: to avoid using the C standard library that has
  // given us many headaches on Windows because of parameter validation we
  // go for a fully C++11 solution based on <chrono> and on the C++11
  // HowardHinnant/date library, which will be available as part of the
  // C++ standard library starting from C++20.
  //
  // Explanation of the algorithm:
  //
  // 1. get the current system time
  // 2. round the time point obtained in the previous step to an integral
  //    number of seconds since the EPOCH used by the system clock
  // 3. create a system clock time point from the integral number of seconds
  // 4. convert the previous result to string using HowardInnant/date
  // 5. if there is a decimal component (there should be one given how the
  //    library we use works) remove it, because OONI doesn't like it
  //
  // (There was another way to deal with fractionary seconds, i.e. using '%OS',
  //  but this solution seems better to me because it's less obscure.)
  using namespace std::chrono;
  constexpr auto fmt = "%Y-%m-%d %H:%M:%S";
  auto sys_point = system_clock::now();                                    // 1
  auto as_seconds = duration_cast<seconds>(sys_point.time_since_epoch());  // 2
  auto back_as_sys_point = system_clock::time_point(as_seconds);           // 3
  auto s = date::format(fmt, back_as_sys_point);                           // 4
  if (s.find(".") != std::string::npos) s = s.substr(0, s.find("."));      // 5
  return s;
}

double monotonic_seconds_now() noexcept {
  auto now = std::chrono::steady_clock::now();
  std::chrono::duration<double> elapsed = now.time_since_epoch();
  return elapsed.count();
}

void Measurement::start() noexcept {
  start_time = ooni_date_now();
  beginning_ = monotonic_seconds_now();
}

void Measurement::stop() noexcept {
  runtime = monotonic_seconds_now() - beginning_;
}

Report::Report() noexcept {}

bool Report::autodiscover_collector(std::vector<std::string> &logs) noexcept {
  // The canonical URL is inside mkbouncer, so let's pass an empty string
  // to signal that we want to use the canonical URL value.
  return autodiscover_collector_with_bouncer("", logs);
}

// append_to_logs appends @p partial to @p full.
static void append_to_logs(std::vector<std::string> &full,
                           std::vector<std::string> &&partial) noexcept {
  full.insert(full.end(), partial.begin(), partial.end());
}

bool Report::autodiscover_collector_with_bouncer(
    const std::string &bouncer_base_url,
    std::vector<std::string> &logs) noexcept {
  mk::bouncer::Request request;
  request.ca_bundle_path = ca_bundle_path;
  if (test_name.empty()) {
    logs.push_back("The test name is empty");
    return false;
  }
  request.name = test_name;
  if (test_version.empty()) {
    logs.push_back("The test version is empty");
    return false;
  }
  request.version = test_version;
  request.timeout = timeout;
  if (!bouncer_base_url.empty()) {
    request.base_url = bouncer_base_url;
  }
  mk::bouncer::Response response = mk::bouncer::perform(request);
  append_to_logs(logs, std::move(response.logs));
  if (!response.good) {
    return false;
  }
  MKREPORT_HOOK(bouncer_response_collectors, response.collectors);
  for (auto &record : response.collectors) {
    if (record.type == "https") {
      collector_base_url = record.address;
      return true;
    }
  }
  logs.push_back("cannot find any suitable https collector");
  return false;
}

bool Report::autodiscover_probe_asn_probe_cc(
    const std::string &asn_db_path, const std::string &country_db_path,
    std::vector<std::string> &logs) noexcept {
  mk::iplookup::Request request;
  request.ca_bundle_path = ca_bundle_path;
  request.timeout = timeout;
  mk::iplookup::Response response = mk::iplookup::perform(request);
  append_to_logs(logs, std::move(response.logs));
  MKREPORT_HOOK(iplookup_response_good, response.good);
  if (!response.good) {
    return false;
  }
  {
    mk::mmdb::Handle handle;
    if (!handle.open(asn_db_path, logs)) {
      return false;
    }
    bool ok = handle.lookup_asn2(response.probe_ip, probe_asn, logs);
    MKREPORT_HOOK(mmdb_asn_lookup, ok);
    if (!ok) {
      return false;
    }
  }
  {
    mk::mmdb::Handle handle;
    if (!handle.open(country_db_path, logs)) {
      return false;
    }
    auto ok = handle.lookup_cc(response.probe_ip, probe_cc, logs);
    MKREPORT_HOOK(mmdb_cc_lookup, ok);
    if (!ok) {
      return false;
    }
  }
  return true;
}

bool Report::open(std::vector<std::string> &logs) noexcept {
  if (!id.empty()) {
    logs.push_back("A report is already open");
    return false;
  }
  mk::collector::OpenRequest request;
  if (probe_asn.empty()) {
    logs.push_back("Please, initialize the probe_asn");
    return false;
  }
  request.probe_asn = probe_asn;
  if (probe_cc.empty()) {
    logs.push_back("Please, initialize the probe_cc.");
    return false;
  }
  request.probe_cc = probe_cc;
  if (software_name.empty()) {
    logs.push_back("Please, initialize the software_name");
    return false;
  }
  request.software_name = software_name;
  if (software_version.empty()) {
    logs.push_back("Please, initialize the software_version");
    return false;
  }
  request.software_version = software_version;
  if (test_name.empty()) {
    logs.push_back("Please, initialize the test_name");
    return false;
  }
  request.test_name = test_name;
  if (test_version.empty()) {
    logs.push_back("Please, initialize the test_version");
    return false;
  }
  request.test_version = test_version;
  if (test_start_time.empty()) {
    logs.push_back("Please, initialize the test_start_time");
    return false;
  }
  request.test_start_time = test_start_time;
  if (collector_base_url.empty()) {
    logs.push_back("Please, initialize the collector_base_url");
    return false;
  }
  request.base_url = collector_base_url;
  request.ca_bundle_path = ca_bundle_path;
  request.timeout = timeout;
  mk::collector::OpenResponse response = mk::collector::open(request);
  append_to_logs(logs, std::move(response.logs));
  std::swap(id, response.report_id);
  return response.good;
}

// submit_measurement_internal submits an already configurd measurement
// by settings its report ID to @p id. It will use the provided @p
// collector_base_url, @p ca_bundle_path, and @p timeout to configure
// the collector client. Returns a boolean and, as usual, stores in
// @p logs any relevant diagnostic information.
static bool submit_measurement_internal(
    mk::collector::UpdateRequest request, const std::string &id,
    const std::string &collector_base_url, const std::string &ca_bundle_path,
    int64_t timeout, std::vector<std::string> &logs) noexcept {
  if (id.empty()) {
    logs.push_back("No configured report ID.");
    return false;
  }
  request.report_id = id;
  if (collector_base_url.empty()) {
    logs.push_back("No configured collector_base_url.");
    return false;
  }
  request.base_url = collector_base_url;
  request.ca_bundle_path = ca_bundle_path;
  request.timeout = timeout;
  mk::collector::UpdateResponse response = mk::collector::update(request);
  append_to_logs(logs, std::move(response.logs));
  return response.good;
}

bool Report::submit_measurement(
    Measurement measurement, std::vector<std::string> &logs) noexcept {
  mk::collector::UpdateRequest request;
  if (!make_content(measurement, request.content, logs)) {
    return false;
  }
  return submit_measurement_internal(
      std::move(request), id, collector_base_url, ca_bundle_path,
      timeout, logs);
}

bool Report::submit_measurement_json(
    std::string measurement, std::vector<std::string> &logs) noexcept {
  mk::collector::UpdateRequest request;
  std::swap(measurement, request.content);
  return submit_measurement_internal(
      std::move(request), id, collector_base_url, ca_bundle_path,
      timeout, logs);
}

bool Report::close(std::vector<std::string> &logs) noexcept {
  if (id.empty()) {
    logs.push_back("No configured report ID.");
    return false;
  }
  mk::collector::CloseRequest request;
  request.report_id = id;
  if (collector_base_url.empty()) {
    logs.push_back("No configured collector_base_url.");
    return false;
  }
  request.base_url = collector_base_url;
  request.ca_bundle_path = ca_bundle_path;
  request.timeout = timeout;
  mk::collector::CloseResponse response = mk::collector::close(request);
  append_to_logs(logs, std::move(response.logs));
  id = "";  // Clear the report ID as this report is now closed
  return response.good;
}

Report::~Report() noexcept {
  std::vector<std::string> logs;
  (void)close(logs);  // Make sure we always close a report
}

bool Report::make_content(const Measurement &measurement, std::string &content,
                          std::vector<std::string> &logs) noexcept {
  // Step 1: parse test keys and make sure it's a JSON object.
  nlohmann::json tk;
  try {
    tk = nlohmann::json::parse(measurement.test_keys);
  } catch (const std::exception &exc) {
    logs.push_back(exc.what());
    return false;
  }
  if (!tk.is_object()) {
    logs.push_back("Test keys is not a JSON object");
    return false;
  }
  // Step 2: fill the measurement JSON object
  nlohmann::json m;
  m["annotations"] = annotations;
  m["data_format_version"] = "0.2.0";
  // The `id` field is constant for all tests. This field is not used in
  // practice and is poised to be removed by the specification. My previous
  // code used r-lyeh/sole@c61c49f10d to generate a UUID4. However, that
  // library uses std::random_device, which is broken with Mingw [1], so I've
  // taken the decision of not bothering myself with this field here.
  //
  // [1] https://sourceforge.net/p/mingw-w64/bugs/338/
  m["id"] = "bdd20d7a-bba5-40dd-a111-9863d7908572";
  m["input"] = measurement.input;
  m["input_hashes"] = nlohmann::json::array();
  m["measurement_start_time"] = measurement.start_time;
  m["options"] = nlohmann::json::array();
  m["probe_asn"] = probe_asn;
  m["probe_cc"] = probe_cc;
  m["probe_city"] = nullptr;
  m["report_id"] = id;
  m["software_name"] = software_name;
  m["software_version"] = software_version;
  m["test_helpers"] = nlohmann::json::object();
  m["test_keys"] = tk;
  m["test_name"] = test_name;
  m["test_runtime"] = measurement.runtime;
  m["test_start_time"] = test_start_time;
  m["test_version"] = test_version;
  // Step 3: dump the measurement message
  try {
    content = m.dump();
  } catch (const std::exception &exc) {
    logs.push_back(exc.what());
    return false;
  }
  return true;
}

bool resubmit_measurement(
    std::string serialized_json, std::string ca_bundle_path,
    int64_t timeout, std::vector<std::string> &logs, std::string &id) noexcept {
  // Step 1: parse the measurement.
  nlohmann::json doc;
  try {
    doc = nlohmann::json::parse(serialized_json);
  } catch (const std::exception &exc) {
    logs.push_back(exc.what());
    return false;
  }
  // Step 2: initialize a new report from the measurement.
  Report report;
  try {
    doc.at("probe_asn").get_to(report.probe_asn);
    doc.at("probe_cc").get_to(report.probe_cc);
    doc.at("software_name").get_to(report.software_name);
    doc.at("software_version").get_to(report.software_version);
    doc.at("test_name").get_to(report.test_name);
    doc.at("test_version").get_to(report.test_version);
    doc.at("test_start_time").get_to(report.test_start_time);
  } catch (const std::exception &exc) {
    logs.push_back(exc.what());
    return false;
  }
  // Step 3: discover the best collector to use using the bouncer.
  auto ok = report.autodiscover_collector(logs);
  MKREPORT_HOOK(report_autodiscover_collector, ok);
  if (!ok) {
    return false;
  }
  // Step 4: open a new report just for this measurement. (We could choose
  // to have a single report for many measurements, but this seems to be
  // tricky for an app, because they would need to make sure that each entry
  // that they submit to us is in the same report; plus, they could perform
  // such an action already with the existing API provided that they make
  // sure they overwrite the report ID.)
  std::swap(ca_bundle_path, report.ca_bundle_path);
  report.timeout = timeout;
  ok = report.open(logs);
  MKREPORT_HOOK(report_open, ok);
  if (!ok) {
    return false;
  }
  // Step 5: unconditionally overwrite the report ID and record it such
  // that the app can update its internal database.
  MKREPORT_HOOK(report_id, report.id);
  doc["report_id"] = report.id;
  id = report.id;
  // Step 6: reserialize the measurement
  std::string modified_measurement;
  try {
    modified_measurement = doc.dump();
  } catch (const std::exception &exc) {
    logs.push_back(exc.what());
    return false;
  }
  // Step 7: submit the measurement within this report.
  ok = report.submit_measurement_json(std::move(modified_measurement), logs);
  MKREPORT_HOOK(report_submit_measurement_json, ok);
  if (!ok) {
    return false;
  }
  // Step 8: close the report.
  return report.close(logs);
}

}  // namespace report
}  // namespace mk
#endif  // MKREPORT_INLINE_IMPL
#endif  // MEASUREMENT_KIT_MKREPORT_HPP
