// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include <measurement_kit/internal/report/report.h>

#include <stdint.h>
#include <stdlib.h>

#include <measurement_kit/internal/vendor/mkreport.hpp>

struct mk_report_datetime {
  std::string s;
};

mk_report_datetime_t *mk_report_datetime_now() {
  mk_report_datetime_t *datetime = new mk_report_datetime_t;
  datetime->s = mk::report::ooni_date_now();
  return datetime;
}

const char *mk_report_datetime_string(const mk_report_datetime_t *datetime) {
  return (datetime) ? datetime->s.c_str() : nullptr;
}

void mk_report_datetime_delete(mk_report_datetime_t *datetime) {
  delete datetime;
}

double mk_report_monotonic_seconds_now() {
  return mk::report::monotonic_seconds_now();
}

struct mk_report {
  mk::report::Report r;
};

mk_report_t *mk_report_new() { return new mk_report_t; }

void mk_report_add_annotation(
    mk_report_t *report, const char *key, const char *value) {
  if (report && key && value) report->r.annotations[key] = value;
}

void mk_report_set_probe_asn(mk_report_t *report, const char *value) {
  if (report && value) report->r.probe_asn = value;
}

void mk_report_set_probe_cc(mk_report_t *report, const char *value) {
  if (report && value) report->r.probe_cc = value;
}

void mk_report_set_software_name(mk_report_t *report, const char *value) {
  if (report && value) report->r.software_name = value;
}

void mk_report_set_software_version(mk_report_t *report, const char *value) {
  if (report && value) report->r.software_version = value;
}

void mk_report_add_test_helper(
    mk_report_t *report, const char *key, const char *value) {
  if (report && key && value) report->r.test_helpers[key] = value;
}

void mk_report_set_test_name(mk_report_t *report, const char *value) {
  if (report && value) report->r.test_name = value;
}

void mk_report_set_test_start_time(mk_report_t *report, const char *value) {
  if (report && value) report->r.test_start_time = value;
}

void mk_report_set_test_version(mk_report_t *report, const char *value) {
  if (report && value) report->r.test_version = value;
}

void mk_report_delete(mk_report_t *report) { delete report; }

struct mk_report_measurement {
  mk::report::Measurement m;
};

mk_report_measurement_t *mk_report_measurement_new(mk_report_t *report) {
  if (report == nullptr) return nullptr;
  mk_report_measurement_t *measurement = new mk_report_measurement_t;
  measurement->m.report = report->r;
  return measurement;
}

void mk_report_measurement_set_input(
    mk_report_measurement_t *measurement, const char *value) {
  if (measurement && value) measurement->m.input = value;
}

void mk_report_measurement_set_start_time(
    mk_report_measurement_t *measurement, const char *value) {
  if (measurement && value) measurement->m.start_time = value;
}

void mk_report_measurement_set_runtime(
    mk_report_measurement_t *measurement, double value) {
  if (measurement) measurement->m.runtime = value;
}

void mk_report_measurement_set_test_keys(
    mk_report_measurement_t *measurement, const char *value) {
  if (measurement && value) measurement->m.test_keys = value;
}

void mk_report_measurement_delete(mk_report_measurement_t *measurement) {
  delete measurement;
}

struct mk_report_serialize_result {
  int64_t good = false;
  std::string reason;
  std::string json;
};

mk_report_serialize_result_t *mk_report_serialize(
    const mk_report_measurement_t *measurement) {
  if (measurement == nullptr) return nullptr;
  auto result = new mk_report_serialize_result_t;
  result->good = mk::report::dump(measurement->m, result->json, result->reason);
  return result;
}

int64_t mk_report_serialize_result_good(
    const mk_report_serialize_result_t *result) {
  return (result) ? result->good : false;
}

const char *mk_report_serialize_result_reason(
    const mk_report_serialize_result_t *result) {
  return (result) ? result->reason.c_str() : nullptr;
}

const char *mk_report_serialize_result_json(
    const mk_report_serialize_result_t *result) {
  return (result) ? result->json.c_str() : nullptr;
}

void mk_report_serialize_result_delete(mk_report_serialize_result_t *result) {
  delete result;
}
