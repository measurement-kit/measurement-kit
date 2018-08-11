/* Part of Measurement Kit <https://measurement-kit.github.io/>.
   Measurement Kit is free software under the BSD license. See AUTHORS
   and LICENSE for more information on the copying conditions. */
#ifndef MEASUREMENT_KIT_FFI_H
#define MEASUREMENT_KIT_FFI_H

/**
 * \file include/measurement_kit/ffi.h
 *
 * Measurement Kit Foreign-Function-Interface (FFI) API. As a general remark,
 * all function that can take a NULL argument should behave correctly when
 * passed a NULL argument; all functions that may return a NULL pointer can
 * do that, so code defensively!
 *
 * \see https://github.com/measurement-kit/measurement-kit/tree/master/example/ffi for usage examples.
 *
 * \see https://github.com/measurement-kit/measurement-kit/blob/master/include/measurement_kit/README.md for event-specific documentation.
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

/** mk_nettest_start() starts a task with the specified JSON settings. You own
 * the returned task pointer and must mk_task_destroy() it when done. NULL is
 * returned in case we cannot parse the JSON settings (or in case of less
 * likely errors). */
mk_task_t *mk_nettest_start(const char *settings) MK_FFI_NOEXCEPT;

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
}  // extern "C"

// Explanation: Visual Studio does not claim to be fully C++11 compatible.
#if __cplusplus >= 201103L || (defined _MSC_VER && _MSC_VER >= 1900)

#include <memory>

class mk_task_deleter {
  public:
    void operator()(mk_task_t *task) noexcept {
        mk_task_destroy(task);
    }
};
using mk_unique_task = std::unique_ptr<mk_task_t, mk_task_deleter>;

class mk_event_deleter {
  public:
    void operator()(mk_event_t *event) noexcept {
        mk_event_destroy(event);
    }
};
using mk_unique_event = std::unique_ptr<mk_event_t, mk_event_deleter>;

#endif  // __cplusplus >= 201103L || (defined _MSC_VER && _MSC_VER >= 1900)
#endif  /* __cplusplus */
#endif  /* MEASUREMENT_KIT_FFI_H */
