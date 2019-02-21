// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include <measurement_kit/internal/collector/collector.h>

#include <stdint.h>
#include <stdlib.h>

#include <measurement_kit/internal/vendor/mkcollector.hpp>
#include <measurement_kit/internal/vendor/mkdata.hpp>

struct mk_collector_settings {
  mk::collector::Settings s;
};

mk_collector_settings_t *mk_collector_settings_new() {
  return new mk_collector_settings_t;
}

void mk_collector_settings_set_base_url(
    mk_collector_settings_t *settings, const char *value) {
  if (settings && value) settings->s.base_url = value;
}

void mk_collector_settings_set_ca_bundle_path(
    mk_collector_settings_t *settings, const char *value) {
  if (settings && value) settings->s.ca_bundle_path = value;
}

void mk_collector_settings_set_timeout(
    mk_collector_settings_t *settings, int64_t timeout) {
  if (settings) settings->s.timeout = timeout;
}

void mk_collector_settings_delete(mk_collector_settings_t *settings) {
  delete settings;
}

struct mk_collector_open_request {
  mk::collector::OpenRequest r;
};

mk_collector_open_request_t *mk_collector_open_request_new() {
  return new mk_collector_open_request_t;
}

void mk_collector_open_request_set_probe_asn(
    mk_collector_open_request_t *request, const char *value) {
  if (request && value) request->r.probe_asn = value;
}

void mk_collector_open_request_set_probe_cc(
    mk_collector_open_request_t *request, const char *value) {
  if (request && value) request->r.probe_cc = value;
}

void mk_collector_open_request_set_software_name(
    mk_collector_open_request_t *request, const char *value) {
  if (request && value) request->r.software_name = value;
}

void mk_collector_open_request_set_software_version(
    mk_collector_open_request_t *request, const char *value) {
  if (request && value) request->r.software_version = value;
}

void mk_collector_open_request_set_test_name(
    mk_collector_open_request_t *request, const char *value) {
  if (request && value) request->r.test_name = value;
}

void mk_collector_open_request_set_test_start_time(
    mk_collector_open_request_t *request, const char *value) {
  if (request && value) request->r.test_start_time = value;
}

void mk_collector_open_request_set_test_version(
    mk_collector_open_request_t *request, const char *value) {
  if (request && value) request->r.test_version = value;
}

void mk_collector_open_request_delete(mk_collector_open_request_t *request) {
  delete request;
}

struct mk_collector_load_open_request {
  mk::collector::LoadResult<mk::collector::OpenRequest> m;
};

mk_collector_load_open_request_t *mk_collector_load_open_request(
    const char *measurement) {
  if (measurement == nullptr) return nullptr;
  auto load = new mk_collector_load_open_request_t;
  load->m = mk::collector::open_request_from_measurement(measurement);
  return load;
}

int64_t mk_collector_load_open_request_good(
    const mk_collector_load_open_request_t *load) {
  return (load) ? load->m.good : false;
}

const char *mk_collector_load_open_request_reason(
    const mk_collector_load_open_request_t *load) {
  return (load) ? load->m.reason.c_str() : nullptr;
}

mk_collector_open_request_t *mk_collector_load_open_request_value(
    const mk_collector_load_open_request_t *load) {
  if (!load) return nullptr;
  auto request = new mk_collector_open_request_t;
  request->r = load->m.value;
  return request;
}

void mk_collector_load_open_request_delete(
    mk_collector_load_open_request_t *load) {
  delete load;
}

struct mk_collector_open_response {
  mk::collector::OpenResponse r;
};

int64_t mk_collector_open_response_good(
    const mk_collector_open_response_t *response) {
  return (response) ? response->r.good : false;
}

const char *mk_collector_open_response_report_id(
    const mk_collector_open_response_t *response) {
  return (response) ? response->r.report_id.c_str() : nullptr;
}

size_t mk_collector_open_response_logs_size(
    const mk_collector_open_response_t *response) {
  return (response) ? response->r.logs.size() : 0;
}

const char *mk_collector_open_response_logs_at(
    const mk_collector_open_response_t *response, size_t idx) {
  return (response && idx < response->r.logs.size()) ?
      response->r.logs[idx].c_str() : nullptr;
}

void mk_collector_open_response_delete(
    mk_collector_open_response_t *response) {
  delete response;
}

static void sanitize_logs(std::vector<std::string> &logs) noexcept {
  for (auto &s : logs) {
    if (!mk::data::contains_valid_utf8(s)) {
      s = mk::data::base64_encode(std::move(s));
    }
  }
}

mk_collector_open_response_t *mk_collector_open(
    const mk_collector_open_request_t *request,
    const mk_collector_settings_t *settings) {
  if (!request || !settings) return nullptr;
  auto response = new mk_collector_open_response_t;
  response->r = mk::collector::open(request->r, settings->s);
  sanitize_logs(response->r.logs);
  return response;
}

struct mk_collector_update_request {
  mk::collector::UpdateRequest r;
};

mk_collector_update_request_t *mk_collector_update_request_new() {
  return new mk_collector_update_request_t;
}

void mk_collector_update_request_set_report_id(
    mk_collector_update_request_t *request, const char *value) {
  if (request && value) request->r.report_id = value;
}

void mk_collector_update_request_set_content(
    mk_collector_update_request_t *request, const char *value) {
  if (request && value) request->r.content = value;
}

void mk_collector_update_request_delete(
    mk_collector_update_request_t *request) {
  delete request;
}

struct mk_collector_update_response {
  mk::collector::UpdateResponse r;
};

int64_t mk_collector_update_response_good(
    const mk_collector_update_response_t *response) {
  return (response) ? response->r.good : false;
}

size_t mk_collector_update_response_logs_size(
    const mk_collector_update_response_t *response) {
  return (response) ? response->r.logs.size() : 0;
}

const char *mk_collector_update_response_logs_at(
    const mk_collector_update_response_t *response, size_t idx) {
  return (response && idx < response->r.logs.size()) ?
    response->r.logs[idx].c_str() : nullptr;
}

void mk_collector_update_response_delete(
    mk_collector_update_response_t *response) {
  delete response;
}

mk_collector_update_response_t *mk_collector_update(
    const mk_collector_update_request_t *request,
    const mk_collector_settings_t *settings) {
  if (!request || !settings) return nullptr;
  auto response = new mk_collector_update_response_t;
  response->r = mk::collector::update(request->r, settings->s);
  sanitize_logs(response->r.logs);
  return response;
}

struct mk_collector_close_request {
  mk::collector::CloseRequest r;
};

mk_collector_close_request_t *mk_collector_close_request_new() {
  return new mk_collector_close_request_t;
}

void mk_collector_close_request_set_report_id(
    mk_collector_close_request_t *request, const char *value) {
  if (request && value) request->r.report_id = value;
}

void mk_collector_close_request_delete(
    mk_collector_close_request_t *request) {
  delete request;
}

struct mk_collector_close_response {
  mk::collector::CloseResponse r;
};

int64_t mk_collector_close_response_good(
    const mk_collector_close_response_t *response) {
  return (response) ? response->r.good : false;
}

size_t mk_collector_close_response_logs_size(
    const mk_collector_close_response_t *response) {
  return (response) ? response->r.logs.size() : 0;
}

const char *mk_collector_close_response_logs_at(
    const mk_collector_close_response_t *response, size_t idx) {
  return (response && idx < response->r.logs.size()) ?
    response->r.logs[idx].c_str() : nullptr;
}

void mk_collector_close_response_delete(
    mk_collector_close_response_t *response) {
  delete response;
}

mk_collector_close_response_t *mk_collector_close(
    const mk_collector_close_request_t *request,
    const mk_collector_settings_t *settings) {
  if (!request || !settings) return nullptr;
  auto response = new mk_collector_close_response_t;
  response->r = mk::collector::close(request->r, settings->s);
  sanitize_logs(response->r.logs);
  return response;
}
