/* Part of Measurement Kit <https://measurement-kit.github.io/>.
   Measurement Kit is free software under the BSD license. See AUTHORS
   and LICENSE for more information on the copying conditions. */
#ifndef MEASUREMENT_KIT_FFI_H
#define MEASUREMENT_KIT_FFI_H

/**
    @file measurement_kit/ffi.h

    @brief Measurement Kit Foreign-Function-Interface (FFI) API.

    The FFI API is the basic API exposed by Measurement Kit. This is a C++ API
    callable from <a href="https://en.wikipedia.org/wiki/Foreign_function_interface">
    FFI</a> that is meant to be stable across releases of the
    Measurement Kit library. Please, keep in mind the following points when
    reasoning about this FFI API:

    1. everything that is not documented, even if it is exposed via a public
       header file, SHOULD be considered as internal and MAY change without
       further notice;

    2. this is a low-level API, hence:
    
       - we will tell you when you have ownership of the returned pointers;
       
       - functions that CAN return NULL MAY return NULL;

       - functions that MAY receive a NULL argument will typically behave
         correctly when passed a NULL argument;

    3. there are more high-level APIs for specific languages, as documented
       in the <a href="https://github.com/measurement-kit/measurement-kit/blob/master/include/measurement_kit/README.md">
       FFI API specification</a>.

    Also, note that in this documentation we are going to reference concepts
    such as _tasks_, _task settings_, _events_, etc. that are defined
    <a href="https://github.com/measurement-kit/measurement-kit/blob/master/include/measurement_kit/README.md">
    in the FFI API specification</a>.

    @addtogroup FFI FFI API
    @brief Foreign Function Interface friendly API.
    @{
*/

#include <stddef.h>

/** @brief Declares that a function does not throw. */
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

/**
    @brief A task that Measurement Kit can run.

    You create a task using mk_task_start(). This function receives as
    argument a serialized JSON containing the task settings. The minimal
    settings JSON contains just the task name, as show in this example:

    ```
    {"name": "Ndt"}
    ```
    
    Please, see <a href="https://github.com/measurement-kit/measurement-kit/blob/master/include/measurement_kit/README.md">
    the FFI API specification</a> for more information regarding the
    schema of the settings as well as on the available task names.

    Creating a Task also creates the thread that will run it. Altough you can
    construct more than one Task at a time, Measurement Kit will make sure that
    tasks do not run concurrently, even though there is no guarantee that they
    will actually run in FIFO order.

    A Task will emit events while running, which you can retrieve using the
    mk_task_wait_for_next_event() call, which blocks until an event occurs. You
    can configure a Task to disable some or all events.

    To know whether a task has finished running, use mk_task_is_done(). This
    will return true when the task thread has exited _and_ there are no unread
    events in the queue drained by mk_task_wait_for_next_event().

    You can also interrupt a running task using mk_task_interrupt().

    The destructor, mk_task_destroy(), will join on the Task thread. That is, it
    will wait for the Task to complete before destroying all the resources.
*/
typedef struct mk_task_ mk_task_t;

/**
    @brief An event emitted by a task.
    
    You can JSON serialize an event using mk_event_serialize(). See the
    <a href="https://github.com/measurement-kit/measurement-kit/blob/master/include/measurement_kit/README.md">
    FFI API specification</a> for more information on the available events type
    and on the JSON schema used by each event.
*/
typedef struct mk_event_ mk_event_t;

/**
    @brief Returns the JSON serialization of an event.

    @param event The event to serialize as JSON.

    @return a serialized event on success.

    @return NULL on failure.

    @return NULL when @p event is NULL.

    @remark The returned string is owned by @p event, DO NOT free it.

    @see <a href="https://github.com/measurement-kit/measurement-kit/blob/master/include/measurement_kit/README.md">
    the FFI API specification</a> for more information on the JSON schema
    used by returned events.
*/
const char *mk_event_serialize(mk_event_t *event) MK_FFI_NOEXCEPT;

/**
    @brief Destroys an event.

    @param event The event to destroy.

    @remark calling this function multiple times on the same @p event
    is a double free memory error with undefined consequences.

    @remark if @p event is NULL this function does nothing.
*/
void mk_event_destroy(mk_event_t *event) MK_FFI_NOEXCEPT;

/**
    @brief Starts a task with the specified JSON settings.

    @param settings A valid serialized JSON containing settings.
    
    @remark You own the returned task pointer and must call mk_task_destroy()
    on it to free resources when done with the task.

    @remark To get more precise error information, use mk_task_start_ex().

    @return a valid task pointer on success.

    @return NULL if it cannot parse the JSON settings.

    @return NULL if @p settings is NULL.

    @return NULL in case of internal error.

    @see <a href="https://github.com/measurement-kit/measurement-kit/blob/master/include/measurement_kit/README.md">
    the FFI API specification</a> for more information on @p settings.
*/
mk_task_t *mk_task_start(const char *settings) MK_FFI_NOEXCEPT;

/** @brief Enumerates the possible error codes returned by
   the mk_task_start_ex() factory function. */
enum mk_task_error_t {
    /** @brief MK_TASK_ENONE indicates that no error occurred. */
    MK_TASK_ENONE = 0,
    /** @brief MK_TASK_EPARSE indicates that we could not parse the settings
        string provided in input as a valid JSON. */
    MK_TASK_EPARSE,
    /** @brief MK_TASK_EGENERIC indicates any other error. */
    MK_TASK_EGENERIC
};

/**
    @brief Starts a task with the specified JSON settings.

    @param task Pointer to pointer to the task. `*task` will contain a valid
    task on success. On failure, `*task` will be `NULL`.

    @param settings Task settings.
    
    @return MK_TASK_EGENERIC if either @p task or @p settings are `NULL`.
    
    @return MK_TASK_EPARSE if @p settings cannot be parsed.
    
    @return MK_TASK_EGENERIC in case of other, unlikely, errors.
    
    @return MK_TASK_ENONE on success.
    
    @remark you should consider @p task to contain a valid pointer, that you own
    and must mk_task_destroy(), only in the MK_TASK_ENONE case.

    @see <a href="https://github.com/measurement-kit/measurement-kit/blob/master/include/measurement_kit/README.md">
    the FFI API specification</a> for more information on @p settings.
*/
enum mk_task_error_t mk_task_start_ex(
        mk_task_t **task, const char *settings) MK_FFI_NOEXCEPT;

/**
    @brief Returns the next event emitted by @p task.

    @param task The task for which you want events. This may be a NULL
    pointer. In such case, the code will return the dummy event usually
    returned when a task is done (a NULL task is always done). Also in
    that case, you _of course_ own the returned event pointer (the fact
    that the event is dummy does not meant that it has not been allocated).

    @return A valid event pointer on success.

    @return NULL on failure.

    @remark If the task has done running, a special dummy event will be
    returned as described in the FFI API specification.
    
    @remark You own the returned event pointer and must call mk_event_destroy()
    to free the allocated resources it when done with the event.

    @remark This is a blocking function that does not return until a
    new event is generated by @p task.
*/
mk_event_t *mk_task_wait_for_next_event(mk_task_t *task) MK_FFI_NOEXCEPT;

/**
    @brief Returns true if the task is done, false otherwise.

    @param task The task that you want to know whether it's done.

    @return true if @p task is done or NULL.

    @return false (i.e. `0`) otherwise.
    
    @remark A task is "done" when the task thread has exited and there are
    no unread events in the queue drained by mk_task_wait_for_next_event().
    
    @remark a NULL task will always be considered done.
*/
int mk_task_is_done(mk_task_t *task) MK_FFI_NOEXCEPT;

/**
    @brief Interrupts a task.

    @param task the Task to interrupt.

    @remark if @p task is already terminated, this function does nothing.

    @remark if @p task is NULL, this function does nothing.
*/
void mk_task_interrupt(mk_task_t *task) MK_FFI_NOEXCEPT;

/**
    @brief Waits for task to complete and frees resources.

    @param task The task to destroy.

    @remark if @p task is NULL, this function does nothing.

    @remark calling this function multiple times on the same @p task
    is a double free memory error with undefined consequences.
*/
void mk_task_destroy(mk_task_t *task) MK_FFI_NOEXCEPT;

#ifdef __cplusplus
}
#endif

/**
    @}
    @mainpage Measurement Kit API

    This is is Measurement Kit documentation generated using Doxygen. It
    documents in details the exported functions, classes, paramters, and
    return values. We encourage you to also read our
    <a href="https://github.com/measurement-kit/measurement-kit/blob/master/include/measurement_kit/README.md">
    top-level API specification</a> to get a better sense of the high
    level design of Measurement Kit APIs.
*/
#endif /* MEASUREMENT_KIT_FFI_H */
