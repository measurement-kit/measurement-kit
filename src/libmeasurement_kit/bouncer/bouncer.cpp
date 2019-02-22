// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include <measurement_kit/internal/bouncer/bouncer.h>

#include <stdint.h>
#include <stdlib.h>

#include <measurement_kit/internal/vendor/mkbouncer.hpp>
#include <measurement_kit/internal/vendor/mkdata.hpp>

const char *mk_bouncer_helper_web_connectivity_id() {
  return MKBOUNCER_HELPER_WEB_CONNECTIVITY;
}

const char *mk_bouncer_helper_tcp_echo_id() {
  return MKBOUNCER_HELPER_TCP_ECHO;
}

const char *mk_bouncer_helper_return_json_headers_id() {
  return MKBOUNCER_HELPER_HTTP_RETURN_JSON_HEADERS;
}

struct mk_bouncer_request {
  mk::bouncer::Request r;
};

mk_bouncer_request_t *mk_bouncer_request_new() {
  auto request = new mk_bouncer_request;
  request->r.name = "generic";
  request->r.version = "0.1.0";
  return request;
}

void mk_bouncer_request_set_base_url(
    mk_bouncer_request_t *request, const char *value) {
  if (request && value) request->r.base_url = value;
}

void mk_bouncer_request_set_ca_bundle_path(
    mk_bouncer_request_t *request, const char *value) {
  if (request && value) request->r.ca_bundle_path = value;
}

void mk_bouncer_request_add_helper_id(
    mk_bouncer_request_t *request, const char *value) {
  if (request && value) request->r.helpers.push_back(value);
}

void mk_bouncer_request_set_nettest_name(
    mk_bouncer_request_t *request, const char *value) {
  if (request && value) request->r.name = value;
}

void mk_bouncer_request_set_nettest_version(
    mk_bouncer_request_t *request, const char *value) {
  if (request && value) request->r.version = value;
}

void mk_bouncer_request_set_timeout(
    mk_bouncer_request_t *request, int64_t value) {
  if (request) request->r.timeout = value;
}

void mk_bouncer_request_delete(mk_bouncer_request_t *request) {
  delete request;
}

struct mk_bouncer_record {
  std::string id;
  std::string type;
  std::string address;
  std::string front;
};

static mk_bouncer_record_t *
mk_bouncer_record_copy_(const mk_bouncer_record_t *record) noexcept {
  if (!record) return nullptr;
  auto copy = new mk_bouncer_record_t;
  *copy = *record;
  return copy;
}

const char *mk_bouncer_record_id(const mk_bouncer_record_t *record) {
  return (record) ? record->id.c_str() : nullptr;
}

const char *mk_bouncer_record_type(const mk_bouncer_record_t *record) {
  return (record) ? record->type.c_str() : nullptr;
}

const char *mk_bouncer_record_address(const mk_bouncer_record_t *record) {
  return (record) ? record->address.c_str() : nullptr;
}

const char *mk_bouncer_record_front(const mk_bouncer_record_t *record) {
  return (record) ? record->front.c_str() : nullptr;
}

void mk_bouncer_record_delete(mk_bouncer_record_t *record) {
  delete record;
}

struct mk_bouncer_response {
  bool good = false;
  std::vector<mk_bouncer_record> records;
  std::vector<std::string> logs;
};

mk_bouncer_response_t *mk_bouncer_query(const mk_bouncer_request_t *request) {
  // TODO(bassosimone): we may want to simplify the implementation of the
  // mkbouncer to avoid having this glue code in here. This can be addressed
  // in a subsequent release of mkbouncer easily.
  if (!request) return nullptr;
  auto r = new mk_bouncer_response_t;
  auto rr = mk::bouncer::perform(request->r);
  r->good = rr.good;
  for (auto &e : rr.collectors) {
    mk_bouncer_record_t record;
    record.id = "collector";
    record.type = e.type;
    record.address = e.address;
    record.front = e.front;
    r->records.push_back(std::move(record));
  }
  for (auto &kvp : rr.helpers) {
    for (auto &entry : kvp.second) {
      mk_bouncer_record_t record;
      record.id = kvp.first;
      record.type = entry.type;
      record.address = entry.address;
      record.front = entry.front;
      r->records.push_back(std::move(record));
    }
  }
  for (auto &s : rr.logs) {
    if (!mk::data::contains_valid_utf8(s)) {
      r->logs.push_back(mk::data::base64_encode(std::move(s)));
      continue;
    }
    r->logs.push_back(s);
  }
  return r;
}

int64_t mk_bouncer_response_good(const mk_bouncer_response_t *response) {
  return (response) ? response->good : false;
}

size_t mk_bouncer_response_records_size(const mk_bouncer_response_t *response) {
  return (response) ? response->records.size() : 0;
}

mk_bouncer_record_t *mk_bouncer_response_records_copy_at(
    const mk_bouncer_response_t *response, size_t idx) {
  return (response && idx < response->records.size()) ?
      mk_bouncer_record_copy_(&response->records[idx]) : nullptr;
}

size_t mk_bouncer_response_logs_size(const mk_bouncer_response_t *response) {
  return (response) ? response->logs.size() : 0;
}

const char *mk_bouncer_response_logs_at(
    const mk_bouncer_response_t *response, size_t idx) {
  return (response && idx < response->logs.size()) ?
      response->logs[idx].c_str() : nullptr;
}

void mk_bouncer_response_delete(mk_bouncer_response_t *response) {
  delete response;
}
