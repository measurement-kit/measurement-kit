/* Part of Measurement Kit <https://measurement-kit.github.io/>.
   Measurement Kit is free software under the BSD license. See AUTHORS
   and LICENSE for more information on the copying conditions. */
#ifndef MEASUREMENT_KIT_BOUNCER_BOUNCER_H
#define MEASUREMENT_KIT_BOUNCER_BOUNCER_H

/** @file measurement_kit/internal/bouncer/bouncer.h
 *
 * @brief Internal FFI API to use OONI bouncer. Beware that internal APIs may
 * change radically at any new release of Measurement Kit.
 */

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/** mk_bouncer_helper_web_connectivity_id returns the ID of the web
 * connectivity helper. This function will return a pointer to a
 * static string, therefore, the return value must not be free()d
 * and is certainly not NULL. */
const char *mk_bouncer_helper_web_connectivity_id(void);

/** mk_bouncer_helper_tcp_echo_id is like mk_bouncer_helper_web_connectivity_id
 * except that it will return the tcp-echo helper ID. */
const char *mk_bouncer_helper_tcp_echo_id(void);

/** mk_bouncer_helper_return_json_headers_id is like
 * mk_bouncer_helper_web_connectivity_id except that it returns the
 * return-json-headers helper ID. */
const char *mk_bouncer_helper_return_json_headers_id(void);

/** mk_bouncer_request_t is a request for the OONI bouncer. */
typedef struct mk_bouncer_request mk_bouncer_request_t;

/** mk_bouncer_request_new creates a new bouncer request. The return value
 * may be NULL. If not NULL, you'll own it and must mk_bouncer_request_delete
 * it when you're done with using it. */
mk_bouncer_request_t *mk_bouncer_request_new(void);

/** mk_bouncer_request_set_base_url sets the bouncer base URL to be @p value
 * instead of being the default bouncer base URL. This function will do nothing
 * if either @p request or @p value are NULL pointers. */
void mk_bouncer_request_set_base_url(
    mk_bouncer_request_t *request, const char *value);

/** mk_bouncer_request_set_ca_bundle_path sets the CA bundle path. This is
 * required on mobile, where there is no system wide CA bundle, and is optional
 * on desktop. This function will do nothing if passed NULL arguments. */
void mk_bouncer_request_set_ca_bundle_path(
    mk_bouncer_request_t *request, const char *value);

/** mk_bouncer_request_add_helper_id adds the helper with @p value ID to the
 * set of helpers that we want to retrieve from the bouncer. The default is that
 * we'll just request for collectors. This function gracefully handles NULL
 * pointer arguments by simply doing nothing. This API include functions
 * like mk_bouncer_helper_tcp_echo_id that allow you to get the canonical ID
 * of test helpers in a programmatic way. */
void mk_bouncer_request_add_helper_id(
    mk_bouncer_request_t *request, const char *value);

/** mk_bouncer_request_set_nettest_name sets the name of the nettest for
 * which we query the bouncer. If this value is not set, we send a generic
 * unspecified nettest name that should work in most cases. This function
 * will gracefully handle any NULL pointer argument. */
void mk_bouncer_request_set_nettest_name(
    mk_bouncer_request_t *request, const char *value);

/** mk_bouncer_request_set_nettest_version sets the version of the nettest for
 * which we query the bouncer. If this value is not set, we send a generic
 * unspecified nettest version that should work in most cases. This function
 * will gracefully handle any NULL pointer argument. */
void mk_bouncer_request_set_nettest_version(
    mk_bouncer_request_t *request, const char *value);

/** mk_bouncer_request_set_timeout sets the time (in seconds) after which
 * a non-complete HTTP request-response cycle is aborted. A zero or negative
 * timeout disables any timeout. The default timeout is an unspecified,
 * positive number of seconds. This function will behave correctly if the
 * @p request argument is NULL; it will just do nothing. */
void mk_bouncer_request_set_timeout(
    mk_bouncer_request_t *request, int64_t value);

/** mk_bouncer_request_delete deletes @p request. Note that @p request
 * may safely be a NULL pointer. */
void mk_bouncer_request_delete(mk_bouncer_request_t *request);

/** mk_bouncer_record_t is a record returned by the OONI bouncer. */
typedef struct mk_bouncer_record mk_bouncer_record_t;

/** mk_bouncer_record_id returns the ID of the @p record. This would be
 * "collector" if @p record describes a collector; otherwise it would
 * be the ID of a specific test helper. The return value MAY be NULL, e.g.
 * when the supplied @p record argument NULL. */
const char *mk_bouncer_record_id(const mk_bouncer_record_t *record);

/** mk_bouncer_record_type returns the type of @p record. In MK we only
 * typically use HTTPS types, which have `"https"` type, but also the
 * `"onion"`, and `"cloudfront"` types are returned by this API. The return
 * value may be NULL (e.g. when @p record is NULL). When it is not NULL it
 * is a pointer to a C string owned by @p request and having the same
 * lifecycle of @p request, so you MUST NOT attempt to free it. */
const char *mk_bouncer_record_type(const mk_bouncer_record_t *record);

/** mk_bouncer_record_address returns the address of @p record. In case of
 * `"https"` records, this is the URL. The same caveats described for
 * mk_bouncer_record_type also apply to this function. */
const char *mk_bouncer_record_address(const mk_bouncer_record_t *record);

/** mk_bouncer_record_front returns the front of @p record. This is empty
 * for all record types, except `"cloudfront"`. The same caveats described
 * for mk_bouncer_record_type also apply to this function. */
const char *mk_bouncer_record_front(const mk_bouncer_record_t *record);

/** mk_bouncer_record_delete deletes @p record. Note that this function
 * gracefully handles the case where you pass it a NULL @p record. */
void mk_bouncer_record_delete(mk_bouncer_record_t *record);

/** mk_bouncer_response_t is the response that you obtain when you
 * query the bouncer using a specific request. */
typedef struct mk_bouncer_response mk_bouncer_response_t;

/** mk_bouncer_query queries the OONI bouncer using the parameters in @p
 * request and returns the result. The return value may be NULL, e.g. when
 * the @p request is NULL. If the return value is not NULL, you own it
 * and must mk_bouncer_response_delete it when done. */
mk_bouncer_response_t *mk_bouncer_query(const mk_bouncer_request_t *request);

/** mk_bouncer_response_good returns zero if mk_bouncer_query failed and
 * nonzero otherwise. Note that the return value will always be zero if
 * you pass to this function a NULL @p response argument. */
int64_t mk_bouncer_response_good(const mk_bouncer_response_t *response);

/** mk_bouncer_response_records_size returns the size of the records
 * array contained in @p response. The return value will always be zero
 * if you pass a NULL pointer argument to this function. */
size_t mk_bouncer_response_records_size(const mk_bouncer_response_t *response);

/** mk_bouncer_response_records_copy_at returns a copy of the record at
 * index @p idx of the @p response records array. The return value may
 * be NULL on failure, as well as if @p response is NULL and/or if @p idx
 * is out of the bounds of the internal records array. You own the returned
 * value, which is a copy of an internal record, therefore, you should
 * mk_bouncer_record_delete it when done. */
mk_bouncer_record_t *mk_bouncer_response_records_copy_at(
    const mk_bouncer_response_t *response, size_t idx);

/** mk_bouncer_response_logs_size returns the size of the logs array
 * owned by @p response. The return value may be zero, for example, if
 * the @p response argument is a NULL pointer. */
size_t mk_bouncer_response_logs_size(const mk_bouncer_response_t *response);

/** mk_bouncer_response_logs_at returns @p response's log entry at index
 * @p idx. The return value is NULL if, for example, @p response is
 * NULL or @p idx is out of bounds. The returned strings is guaranteed
 * to be UTF-8; in case the log is not UTF-8, we will return you the
 * base64 of the specific log entry. */
const char *mk_bouncer_response_logs_at(
    const mk_bouncer_response_t *response, size_t idx);

/** mk_bouncer_response_delete deletes @p response. Note that the @p
 * response argument may safely be NULL. */
void mk_bouncer_response_delete(mk_bouncer_response_t *response);

#ifdef __cplusplus
}
#endif  /* __cplusplus */
#endif  /* MEASUREMENT_KIT_BOUNCER_BOUNCER_H */
