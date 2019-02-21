/* Part of Measurement Kit <https://measurement-kit.github.io/>.
   Measurement Kit is free software under the BSD license. See AUTHORS
   and LICENSE for more information on the copying conditions. */
#ifndef MEASUREMENT_KIT_REPORT_REPORT_H
#define MEASUREMENT_KIT_REPORT_REPORT_H

/** @file measurement_kit/internal/report/report.h
 *
 * @brief Internal FFI API to use OONI report. Beware that internal APIs may
 * change radically at any new release of Measurement Kit.
 */

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/** mk_report_datetime_t contains the current date and time in UTC formatted
 * using the "%Y-%m-%d %H:%M:%S" format string. */
typedef struct mk_report_datetime mk_report_datetime_t;

/** mk_report_datetime_now returns the current date and time in the
 * moment in which the function is called. It may return a NULL pointer
 * on failure. If the return value is not NULL, you own the returned
 * pointer and must mk_report_datetime_delete it when done. */
mk_report_datetime_t *mk_report_datetime_now(void);

/** mk_report_datetime_string returns a string representation of @p
 * datetime. The return value may be NULL on failure, e.g. when this
 * function is provided with a NULL argument. Otherwise, it is a C
 * string owned by @p datetime and having the same lifecycle. */
const char *mk_report_datetime_string(const mk_report_datetime_t *datetime);

/** mk_report_datetime_delete deletes @p datetime. If @p datetime is
 * a NULL pointer, this function will do nothing. */
void mk_report_datetime_delete(mk_report_datetime_t *datetime);

/** mk_report_monotonic_seconds_now() returns the number of seconds elapsed
 * since the zero of the C++11 monotonic clock. */
double mk_report_monotonic_seconds_now(void);

/** mk_report_t is a OONI report. It contains common variables that are
 * shared by all the measurements within a report. */
typedef struct mk_report mk_report_t;

/** mk_report_new creates a new, empty OONI report. The return value may be
 * NULL on failure. If it's not NULL, you own the returned pointer and you
 * must mk_report_delete it when you are done using it. */
mk_report_t *mk_report_new(void);

/** mk_report_add_annotation adds an annotation to a report. This function
 * will do nothing if any input pointer is NULL. */
void mk_report_add_annotation(
    mk_report_t *report, const char *key, const char *value);

/** mk_report_set_probe_asn sets the probe ASN. This function will do nothing
 * if any input pointer is a NULL pointer. */
void mk_report_set_probe_asn(mk_report_t *report, const char *value);

/** mk_report_set_probe_cc sets the probe CC. This function will do nothing
 * if any input pointer is a NULL pointer. */
void mk_report_set_probe_cc(mk_report_t *report, const char *value);

/** mk_report_set_software_name sets the software name. This function will do
 * nothing if any input pointer is a NULL pointer. */
void mk_report_set_software_name(mk_report_t *report, const char *value);

/** mk_report_set_software_version sets the software version. This function
 * will do nothing if any input pointer is a NULL pointer. */
void mk_report_set_software_version(mk_report_t *report, const char *value);

/** mk_report_add_test_helper adds a test helper to the report. This function
 * will do nothing if any input pointer is a NULL pointer. */
void mk_report_add_test_helper(
    mk_report_t *report, const char *key, const char *value);

/** mk_report_set_test_name sets the test name. This function
 * will do nothing if any input pointer is a NULL pointer. */
void mk_report_set_test_name(mk_report_t *report, const char *value);

/** mk_report_set_test_start_time sets the test start time. The @p value
 * may be filled in using mk_report_datetime_t's API. This function will do
 * nothing if any input pointer is a NULL pointer. */
void mk_report_set_test_start_time(mk_report_t *report, const char *value);

/** mk_report_set_test_version sets the test version. This function
 * will do nothing if any input pointer is a NULL pointer. */
void mk_report_set_test_version(mk_report_t *report, const char *value);

/** mk_report_delete deletes @p report. This function will do
 * nothing if @p report is NULL. */
void mk_report_delete(mk_report_t *report);

/** mk_report_measurement_t is a measurement within a OONI report. */
typedef struct mk_report_measurement mk_report_measurement_t;

/** mk_report_measurement_new creates a new measurement from the provided
 * @p report. This function will copy @p report, therefore the returned
 * pointer and @p report may have different lifecycles. This function may
 * return NULL on failure, e.g., if @p report is NULL. If the return value
 * isn't NULL, you must mk_report_measurement_delete it when done. */
mk_report_measurement_t *mk_report_measurement_new(mk_report_t *report);

/** mk_report_measurement_set_input sets the measurement input. This is
 * only required if the test takes any input. This function will do nothing
 * if any input argument is NULL. */
void mk_report_measurement_set_input(
    mk_report_measurement_t *measurement, const char *value);

/** mk_report_measurement_set_start_time sets the measurement start time. The
 * @p value may be filled in using mk_report_datetime_t's API. This function
 * will do nothing if any input pointer is a NULL pointer. */
void mk_report_measurement_set_start_time(
    mk_report_measurement_t *measurement, const char *value);

/** mk_report_measurement_set_runtime sets the measurement runtime. The
 * @p value may be filled in using mk_report_monotonic_seconds_now's API
 * to measure the elapsed time between the beginning and the end of the
 * measurement. This function will do nothing if @p measurement is NULL. */
void mk_report_measurement_set_runtime(
    mk_report_measurement_t *measurement, double value);

/** mk_report_measurement_set_test_keys sets the measurement test keys. The
 * @p value must be a valid serialized JSON object containing the results
 * of a measurement. This function will gracefully do nothing if any input
 * pointer is a NULL pointer. */
void mk_report_measurement_set_test_keys(
    mk_report_measurement_t *measurement, const char *value);

/** mk_report_measurement_delete deletes a @p measurement. This function will
 * do nothing if @p measurement is a NULL pointer. */
void mk_report_measurement_delete(mk_report_measurement_t *measurement);

/** mk_report_serialize_result_t contains the result of attempting
 * to serialize a measurement to JSON. */
typedef struct mk_report_serialize_result mk_report_serialize_result_t;

/** mk_report_serialize serializes @p measurement and returns the result. It
 * returns NULL on failure, e.g., if @p measurement is NULL. If the return
 * value isn't NULL, you must mk_report_serialize_result_delete it when done. */
mk_report_serialize_result_t *mk_report_serialize(
    const mk_report_measurement_t *measurement);

/** mk_report_serialize_result_good returns zero if the serialization failed
 * and nonzero otherwise. The return value is zero if @p result is NULL. */
int64_t mk_report_serialize_result_good(
    const mk_report_serialize_result_t *result);

/** mk_report_serialize_result_reason returns the reason why serialization
 * failed. The return value is NULL if @p result is NULL, and an empty string
 * if no error occurred. The return value is owned by @p result. */
const char *mk_report_serialize_result_reason(
    const mk_report_serialize_result_t *result);

/** mk_report_serialize_result_json returns the JSON serialized measurement. If
 * @p result is NULL, and under other error conditions, the return value will
 * be NULL. Otherwise, it's a C string owned by @p result. Such string will be
 * empty if serialization failed, otherwise it will be a parseable JSON. */
const char *mk_report_serialize_result_json(
    const mk_report_serialize_result_t *result);

/** mk_report_serialize_result_delete deletes @p result. If @p result is
 * a NULL pointer, this function will do nothing. */
void mk_report_serialize_result_delete(mk_report_serialize_result_t *result);

#ifdef __cplusplus
}
#endif  /* __cplusplus */
#endif  /* MEASUREMENT_KIT_REPORT_REPORT_H */
