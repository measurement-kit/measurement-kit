// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include <measurement_kit/internal/geoiplookup/geoiplookup.h>

#include <stdint.h>
#include <stdlib.h>

#include <measurement_kit/internal/vendor/mkdata.hpp>
#include <measurement_kit/internal/vendor/mkgeoip.hpp>

struct mk_geoiplookup_request {
  mk::geoip::LookupSettings r;
};

mk_geoiplookup_request_t *mk_geoiplookup_request_new() {
  return new mk_geoiplookup_request;
}

void mk_geoiplookup_request_set_ca_bundle_path(
    mk_geoiplookup_request_t *request, const char *value) {
  if (request && value) request->r.ca_bundle_path = value;
}

void mk_geoiplookup_request_set_asn_db_path(
    mk_geoiplookup_request_t *request, const char *value) {
  if (request && value) request->r.asn_db_path = value;
}

void mk_geoiplookup_request_set_country_db_path(
    mk_geoiplookup_request_t *request, const char *value) {
  if (request && value) request->r.country_db_path = value;
}

void mk_geoiplookup_request_set_timeout(
    mk_geoiplookup_request_t *request, int64_t value) {
  if (request) request->r.timeout = value;
}

void mk_geoiplookup_request_delete(mk_geoiplookup_request_t *request) {
  delete request;
}

struct mk_geoiplookup_response {
  mk::geoip::LookupResults r;
};

mk_geoiplookup_response_t *mk_geoiplookup_perform(
    const mk_geoiplookup_request_t *request) {
  if (request == nullptr) return nullptr;
  auto response = new mk_geoiplookup_response_t;
  response->r = mk::geoip::lookup(request->r);
  if (!response->r.good) {
    // Implementation note: the logic used by mk::geoip::lookup to determine
    // success checks the values of the probe_ip, probe_asn, etc fields. This
    // means we cannot initialize the results to the values we want to have
    // back on failure ("ZZ", "AS0", etc). We should instead check whether we
    // are good and set them afterwards. This glue code can probably go, if
    // we include this logic into the next release of mkgeoip.
    if (response->r.probe_asn_string.empty()) {
      response->r.probe_asn_string = "AS0";
    }
    if (response->r.probe_cc.empty()) {
      response->r.probe_cc = "ZZ";
    }
  }
  for (auto &s : response->r.logs) {
    if (!mk::data::contains_valid_utf8(s)) {
      s = mk::data::base64_encode(std::move(s));
    }
  }
  return response;
}

int64_t mk_geoiplookup_response_good(
    const mk_geoiplookup_response_t *response) {
  return (response) ? response->r.good : false;
}

const char *mk_geoiplookup_response_ip(
    const mk_geoiplookup_response_t *response) {
  return (response) ? response->r.probe_ip.c_str() : nullptr;
}

const char *mk_geoiplookup_response_asn(
    const mk_geoiplookup_response_t *response) {
  return (response) ? response->r.probe_asn_string.c_str() : nullptr;
}

const char *mk_geoiplookup_response_cc(
    const mk_geoiplookup_response_t *response) {
  return (response) ? response->r.probe_cc.c_str() : nullptr;
}

const char *mk_geoiplookup_response_org(
    const mk_geoiplookup_response_t *response) {
  return (response) ? response->r.probe_org.c_str() : nullptr;
}

size_t mk_geoiplookup_response_logs_size(
    const mk_geoiplookup_response_t *response) {
  return (response) ? response->r.logs.size() : 0;
}

const char *mk_geoiplookup_response_logs_at(
    const mk_geoiplookup_response_t *response, size_t idx) {
  return (response && idx < response->r.logs.size()) ?
      response->r.logs[idx].c_str() : nullptr;
}

void mk_geoiplookup_response_delete(mk_geoiplookup_response_t *response) {
  delete response;
}
