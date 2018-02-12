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
 * the returned task pointer and must mk_task_destroy() it when done. */
mk_task_t *mk_task_start(const char *settings) MK_FFI_NOEXCEPT;

/** mk_task_start_ex() starts a task with the specified JSON settings. It will
 * write any start error into the provided buffer (if not NULL and length is
 * positive, of course). The error buffer will be always zero terminated if its
 * length is at least equal to one character and the buffer is not NULL. You
 * own the returned task pointer and must mk_task_destroy() it when done. */
mk_task_t *mk_task_start_ex(
        const char *settings, char *errbuf, size_t errbuf_size) MK_FFI_NOEXCEPT;

/** mk_task_wait_for_next_event() blocks until the next event. You own the
 * returned event pointer and must mk_event_destroy() it when done. */
mk_event_t *mk_task_wait_for_next_event(mk_task_t *task) MK_FFI_NOEXCEPT;

/** mk_task_is_running() returns nonzero if the task is running, 0 otherwise. */
int mk_task_is_running(mk_task_t *task) MK_FFI_NOEXCEPT;

/** mk_task_interrupt() interrupts a task. */
void mk_task_interrupt(mk_task_t *task) MK_FFI_NOEXCEPT;

/** mk_task_destroy() waits for task to complete and frees resources. */
void mk_task_destroy(mk_task_t *task) MK_FFI_NOEXCEPT;

#ifdef __cplusplus
}
#endif
#endif /* MEASUREMENT_KIT_FFI_H */
