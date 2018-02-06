/* Part of Measurement Kit <https://measurement-kit.github.io/>.
   Measurement Kit is free software under the BSD license. See AUTHORS
   and LICENSE for more information on the copying conditions. */
#ifndef MEASUREMENT_KIT_FFI_H
#define MEASUREMENT_KIT_FFI_H

/*
 * Measurement Kit Foreign-Function-Interface (FFI) API.
 *
 * See include/measurement_kit/engine.h for more documentation.
 *
 * See example/ffi/ndt.cpp for example usage.
 */

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

/** mk_task_start() starts a task with the specified JSON settings. */
mk_task_t *mk_task_start(const char *settings) MK_FFI_NOEXCEPT;

/** mk_task_wait_for_next_event() blocks until the next event. */
mk_event_t *mk_task_wait_for_next_event(mk_task_t *task) MK_FFI_NOEXCEPT;

/** mk_task_interrupt() interrupts a task. */
void mk_task_interrupt(mk_task_t *task) MK_FFI_NOEXCEPT;

/** mk_task_destroy() waits for task to complete and frees resources. */
void mk_task_destroy(mk_task_t *task) MK_FFI_NOEXCEPT;

#ifdef __cplusplus
}
#endif
#endif /* MEASUREMENT_KIT_FFI_H */
