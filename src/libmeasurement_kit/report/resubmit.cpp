// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include <measurement_kit/internal/report/resubmit.h>

#include <stdint.h>
#include <stdlib.h>

#include <algorithm>
#include <string>
#include <vector>

#include <measurement_kit/internal/vendor/mkreport.hpp>

// mk_report_resubmit_request contains the resubmit request params.
struct mk_report_resubmit_request {
  std::string json_str;
  std::string ca_bundle_path;
  int64_t timeout = 30;
};

mk_report_resubmit_request_t *mk_report_resubmit_request_new() {
  return new mk_report_resubmit_request;
}

void mk_report_resubmit_request_set_json_str(
    mk_report_resubmit_request_t *request, const char *value) {
  if (request && value) request->json_str = value;
}

void mk_report_resubmit_request_set_ca_bundle_path(
    mk_report_resubmit_request_t *request, const char *value) {
  if (request && value) request->ca_bundle_path = value;
}

void mk_report_resubmit_request_set_timeout(
    mk_report_resubmit_request_t *request, int64_t value) {
  if (request) request->timeout = value;
}

void mk_report_resubmit_request_delete(mk_report_resubmit_request_t *request) {
  delete request;
}

// mk_report_resubmit_response contains the resubmit response values.
struct mk_report_resubmit_response {
  int64_t good = false;
  std::vector<std::string> logs;
  std::string report_id;
};

mk_report_resubmit_response_t *mk_report_resubmit_movein_and_perform(
    mk_report_resubmit_request_t *request) {
  if (request == nullptr) return nullptr;
  auto response = new mk_report_resubmit_response_t;
  response->good = mk::report::resubmit_measurement(
      std::move(request->json_str), std::move(request->ca_bundle_path),
      request->timeout, response->logs, response->report_id);
  return response;
}

int64_t mk_report_resubmit_response_good(
    const mk_report_resubmit_response_t *response) {
  return (response) ? response->good : false;
}

size_t mk_report_resubmit_response_logs_size(
    const mk_report_resubmit_response_t *response) {
  return (response) ? response->logs.size() : 0;
}

const char *mk_report_resubmit_response_logs_at(
    const mk_report_resubmit_response_t *response, size_t idx) {
  return (response && idx < response->logs.size()) ?
      response->logs[idx].c_str() : nullptr;
}

const char *mk_report_resubmit_response_report_id(
    const mk_report_resubmit_response_t *response) {
  return (response) ? response->report_id.c_str() : nullptr;
}

void mk_report_resubmit_response_delete(
    mk_report_resubmit_response_t *response) {
  delete response;
}
