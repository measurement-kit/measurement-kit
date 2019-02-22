/* Part of Measurement Kit <https://measurement-kit.github.io/>.
   Measurement Kit is free software under the BSD license. See AUTHORS
   and LICENSE for more information on the copying conditions. */
#ifndef MEASUREMENT_KIT_GEOIPLOOKUP_GEOIPLOOKUP_H
#define MEASUREMENT_KIT_GEOIPLOOKUP_GEOIPLOOKUP_H

/** @file measurement_kit/internal/geoiplookup/geoiplookup.h
 *
 * @brief Internal FFI API for geoiplookup. Beware that internal APIs may
 * change radically at any new release of Measurement Kit.
 */

/*
 * Implementation note: this API could have been called geoip. Yet, a previous
 * version of measurement-kit/mkgeoip had a C API and was already used by the
 * apps. So, I wanted to avoid the unfortunate case where the code was compiling
 * because the name were the same, but perhaps the behaviour was different. In
 * such cases, I'd rather break compilation badly, so that we can actually go
 * and read the code and think carefully. That explains the naming.
 */

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/** mk_geoiplookup_request_t contains the settings used to perform a
 * geoiplookup (i.e. a discovery of IP, country code, ASN, etc). */
typedef struct mk_geoiplookup_request mk_geoiplookup_request_t;

/** mk_geoiplookup_request_new creates a new geoiplookup request. The return
 * value may be NULL. If not NULL, you'll own it and
 * must mk_geoiplookup_request_delete it when you're done with using it. */
mk_geoiplookup_request_t *mk_geoiplookup_request_new(void);

/** mk_geoiplookup_request_set_ca_bundle_path sets the CA bundle path. This is
 * required on mobile, where there is no system wide CA bundle, and is optional
 * on desktop. This function will do nothing if passed NULL arguments. */
void mk_geoiplookup_request_set_ca_bundle_path(
    mk_geoiplookup_request_t *request, const char *value);

/** mk_geoiplookup_request_set_asn_db_path sets the MMDB ASN DB path. This is
 * required on all platforms. Not providing this value will make the geoip
 * lookup fail. This function will do nothing if passed NULL arguments. */
void mk_geoiplookup_request_set_asn_db_path(
    mk_geoiplookup_request_t *request, const char *value);

/** mk_geoiplookup_request_set_country_db_path is exactly like
 * mk_geoiplookup_request_set_asn_db_path except that it sets the
 * MMDB country DB path. */
void mk_geoiplookup_request_set_country_db_path(
    mk_geoiplookup_request_t *request, const char *value);

/** mk_geoiplookup_request_set_timeout sets the time (in seconds) after which
 * a non-complete HTTP request-response cycle is aborted. A zero or negative
 * timeout disables any timeout. The default timeout is an unspecified,
 * positive number of seconds. This function will behave correctly if the
 * @p request argument is NULL; it will just do nothing. */
void mk_geoiplookup_request_set_timeout(
    mk_geoiplookup_request_t *request, int64_t value);

/** mk_geoiplookup_request_delete deletes @p request. Note that @p request
 * may safely be a NULL pointer. */
void mk_geoiplookup_request_delete(mk_geoiplookup_request_t *request);

/** mk_geoiplookup_response_t is the response that you obtain when you
 * perform a geoiplookup using a specific request. */
typedef struct mk_geoiplookup_response mk_geoiplookup_response_t;

/** mk_geoiplookup_query performs a geoiplookup using the parameters in @p
 * request, and returns the result. The return value may be NULL, e.g. when
 * the @p request is NULL. If the return value is not NULL, you own it
 * and must mk_geoiplookup_response_delete it when done. */
mk_geoiplookup_response_t *mk_geoiplookup_perform(
    const mk_geoiplookup_request_t *request);

/** mk_geoiplookup_iplookup_good returns zero if mk_geoiplookup_query failed
 * and nonzero otherwise. Note that the return value will always be zero if
 * you pass to this function a NULL @p response argument. */
int64_t mk_geoiplookup_response_good(const mk_geoiplookup_response_t *response);

/** mk_geoip_lookup_ip returns the discovered IP. This function may return NULL
 * e.g. if @p response is NULL. In the common case, the returned value is a
 * valid C string owned by @p response and having the same lifecycle. Note that
 * the return value is an empty string if we could not discover the IP. */
const char *mk_geoiplookup_response_ip(
    const mk_geoiplookup_response_t *response);

/** mk_geoiplookup_response_asn is like mk_geoiplookup_response_ip except
 * that it returns the ASN. Unless the return value is NULL, it is a C
 * string starting with `"AS"` and followed by the AS number. The default
 * value used in case of failure is `"AS0"`. */
const char *mk_geoiplookup_response_asn(
    const mk_geoiplookup_response_t *response);

/** mk_geoiplookup_response_cc is like mk_geoiplookup_response_ip except
 * that it returns the country code. Unless the return value is NULL, it is
 * a valid nonempty C string. The default value used on failure is "ZZ". */
const char *mk_geoiplookup_response_cc(
    const mk_geoiplookup_response_t *response);

/** mk_geoiplookup_response_org is like mk_geoiplookup_response_ip except
 * that it returns the ASN name. The return value is either NULL of a
 * valid C string. We use the empty string on failure. */
const char *mk_geoiplookup_response_org(
    const mk_geoiplookup_response_t *response);

/** mk_geoiplookup_response_logs_size returns the size of the logs array
 * owned by @p response. The return value may be zero, for example, if
 * the @p response argument is a NULL pointer. */
size_t mk_geoiplookup_response_logs_size(
    const mk_geoiplookup_response_t *response);

/** mk_geoiplookup_response_logs_at returns @p response's log entry at index
 * @p idx. The return value is NULL if, for example, @p response is
 * NULL or @p idx is out of bounds. The returned strings is guaranteed
 * to be UTF-8; in case the log is not UTF-8, we will return you the
 * base64 of the specific log entry. */
const char *mk_geoiplookup_response_logs_at(
    const mk_geoiplookup_response_t *response, size_t idx);

/** mk_geoiplookup_response_delete deletes @p response. Note that the @p
 * response argument may safely be NULL. */
void mk_geoiplookup_response_delete(mk_geoiplookup_response_t *response);

#ifdef __cplusplus
}
#endif  /* __cplusplus */
#endif  /* MEASUREMENT_KIT_GEOIPLOOKUP_GEOIPLOOKUP_H */
