/* Part of Measurement Kit <https://measurement-kit.github.io/>.
   Measurement Kit is free software under the BSD license. See AUTHORS
   and LICENSE for more information on the copying conditions. */
#ifndef MEASUREMENT_KIT_REPORT_RESUBMIT_H
#define MEASUREMENT_KIT_REPORT_RESUBMIT_H

/** @file measurement_kit/internal/report/resubmit.h
 *
 * @brief Internal FFI API to resubmit reports. Beware that internal APIs may
 * change radically at any new release of Measurement Kit.
 */

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/** mk_report_resubmit_request_t is a request to resubmit a report. */
typedef struct mk_report_resubmit_request mk_report_resubmit_request_t;

/** mk_report_resubmit_request_new creates a new report resubmit request. It
 * returns a valid pointer on success and NULL on failure. You own the returned
 * pointer and must mk_report_resubmit_request_delete() it when done. */
mk_report_resubmit_request_t *mk_report_resubmit_request_new(void);

/** mk_report_resubmit_request_set_json_str sets the JSON to resubmit. You 
 * MUST set this value otherwise we don't know what to resubmit. If @p request
 * and/or @p value are NULL, this function does nothing. */
void mk_report_resubmit_request_set_json_str(
    mk_report_resubmit_request_t *request, const char *value);

/** mk_report_resubmit_request_set_ca_bundle_path sets the CA bundle path. You
 * will need to set this on mobile. If either argument is NULL, then this
 * function will gracefully do nothing. */
void mk_report_resubmit_request_set_ca_bundle_path(
    mk_report_resubmit_request_t *request, const char *value);

/** mk_report_resubmit_request_set_timeout sets the timeout. Specifically, this
 * is the number of seconds after which the resubmit request is aborted. Note
 * that a zero or negative timeout means no timeout. Not setting any timeout
 * will cause MK to use a reasonable default. If the @p request argument
 * is NULL, then this function will gracefully do nothing. */
void mk_report_resubmit_request_set_timeout(
    mk_report_resubmit_request_t *request, int64_t value);

/** mk_report_resubmit_request_delete destroys a resubmit request. This
 * function will gracefully handle a NULL @p request argument. */
void mk_report_resubmit_request_delete(mk_report_resubmit_request_t *request);

/** mk_report_resubmit_response_t is the response to a resubmit request. */
typedef struct mk_report_resubmit_response mk_report_resubmit_response_t;

/** mk_report_resubmit_movein_and_perform resubmits a report. It will only
 * return NULL either in case of allocation failure or if @p request is
 * NULL. Otherwise, it will return a mk_report_resubmit_response pointer
 * that you own and must mk_report_resubmit_response_delete() when done. It
 * should also be noted that this operation has move semantics: after
 * you've called this function, you should not use @p request again. */
mk_report_resubmit_response_t *mk_report_resubmit_movein_and_perform(
    mk_report_resubmit_request_t *request);

/** mk_report_resubmit_response_good tells you whether we succeeded. The
 * return value is zero on failure, nonzero on success. This function will
 * always return zero if @p response is NULL. */
int64_t mk_report_resubmit_response_good(
    const mk_report_resubmit_response_t *response);

/** mk_report_resubmit_response_logs_size tells you the logs size. This
 * function will always return zero if @p response is NULL. */
size_t mk_report_resubmit_response_logs_size(
    const mk_report_resubmit_response_t *response);

/** mk_report_resubmit_response_logs_at returns a specific log entry. This
 * function will return NULL if @p response is NULL and/or the @p idx index
 * is not smaller than mk_report_resubmit_logs_size(). The returned string
 * is owened by @p response and hence has the same lifecycle. */
const char *mk_report_resubmit_response_logs_at(
    const mk_report_resubmit_response_t *response, size_t idx);

/** mk_report_resubmit_response_report_id tells you the new report ID. This
 * function returns NULL if @p response is NULL. Additionally, it may return
 * an empty string if the operation failed. The returned pointer is owned
 * by @p response and thus has the same lifecycle of it. */
const char *mk_report_resubmit_response_report_id(
    const mk_report_resubmit_response_t *response);

/** mk_report_resubmit_response_delete deletes a response. In case @p
 * response is NULL this function will gracefully do nothing. */
void mk_report_resubmit_response_delete(
    mk_report_resubmit_response_t *response);

#ifdef __cplusplus
}
#endif  /* __cplusplus */
#endif  /* MEASUREMENT_KIT_REPORT_RESUBMIT_H */
