/* Part of Measurement Kit <https://measurement-kit.github.io/>.
   Measurement Kit is free software under the BSD license. See AUTHORS
   and LICENSE for more information on the copying conditions. */
#ifndef MEASUREMENT_KIT_FFI_H
#define MEASUREMENT_KIT_FFI_H

/*
 * Measurement Kit Foreign-Function-Interface (FFI) API. As a general remark,
 * all function that can take a NULL argument should behave correctly when
 * passed a NULL argument; all functions that may return a NULL pointer can
 * do that, so code defensively!
 *
 * See include/measurement_kit/engine.h for more documentation.
 *
 * See example/ffi/ndt.cpp for example usage.
 */

#include <stddef.h>

/** MK_FFI_NOEXCEPT declares that a function does not throw. */
#if defined(__cplusplus) && __cplusplus >= 201103L
#define MK_FFI_NOEXCEPT noexcept
#elif defined(__cplusplus)
#define MK_FFI_NOEXCEPT throw()
#else
#define MK_FFI_NOEXCEPT /* Nothing */
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** mk_event_t is an opaque event emitted by a task. */
typedef struct mk_event_ mk_event_t;

/** mk_event_serialize() obtains a JSON serialization of an event. */
const char *mk_event_serialize(mk_event_t *event) MK_FFI_NOEXCEPT;

/** mk_event_destroy() destroys an event. */
void mk_event_destroy(mk_event_t *event) MK_FFI_NOEXCEPT;

/** mk_task_t is a task that Measurement Kit can run. */
typedef struct mk_task_ mk_task_t;

/** mk_task_start() starts a task with the specified JSON settings. You own
 * the returned task pointer and must mk_task_destroy() it when done. NULL is
 * returned in case we cannot parse the JSON settings (or in case of less
 * likely errors; see mk_task_start_ex()). */
mk_task_t *mk_task_start(const char *settings) MK_FFI_NOEXCEPT;

/** mk_task_error_t enumerates the possible error codes returned by
 * the mk_task_start_ex() factory function. */
enum mk_task_error_t {
    /** MK_TASK_ENONE indicates that no error occurred. */
    MK_TASK_ENONE = 0,
    /** MK_TASK_EPARSE indicates that we could not parse the settings string
     * provided in input as a valid JSON. */
    MK_TASK_EPARSE,
    /** MK_TASK_EGENERIC indicates any other error. */
    MK_TASK_EGENERIC
};

/** mk_task_start_ex() starts a task with the specified JSON settings. @return
 * MK_TASK_EGENERIC if either @p task or @p settings are `NULL`. @return
 * MK_TASK_EPARSE if @p settings cannot be parsed. @return MK_TASK_EGENERIC
 * in case of other, unlikely, errors. @return MK_TASK_ENONE on success. @note
 * you should consider @p task to contain a valid pointer, that you own and
 * must mk_task_destroy(), only in the MK_TASK_ENONE case. */
enum mk_task_error_t mk_task_start_ex(
        mk_task_t **task, const char *settings) MK_FFI_NOEXCEPT;

/** mk_task_wait_for_next_event() blocks until the next event. You own the
 * returned event pointer and must mk_event_destroy() it when done. */
mk_event_t *mk_task_wait_for_next_event(mk_task_t *task) MK_FFI_NOEXCEPT;

/** mk_task_is_done() returns nonzero if the task is done, 0 otherwise. A task
 * is done when the task thread has exited and there are no unread events in
 * the queue drained by mk_task_wait_for_next_event(). @note a NULL task will
 * always be considered done. */
int mk_task_is_done(mk_task_t *task) MK_FFI_NOEXCEPT;

/** mk_task_interrupt() interrupts a task. */
void mk_task_interrupt(mk_task_t *task) MK_FFI_NOEXCEPT;

/** mk_task_destroy() waits for task to complete and frees resources. */
void mk_task_destroy(mk_task_t *task) MK_FFI_NOEXCEPT;

#ifdef __cplusplus
}
#endif
#endif /* MEASUREMENT_KIT_FFI_H */
