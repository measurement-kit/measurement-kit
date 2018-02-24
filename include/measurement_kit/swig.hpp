// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_SWIG_HPP
#define MEASUREMENT_KIT_SWIG_HPP

/*
 * Measurement Kit C++ interface for SWIG.
 *
 * See include/measurement_kit/engine.h for more documentation.
 *
 * See example/swig/ndt.cpp for example.
 */

#include <measurement_kit/ffi.h>

#include <memory>
#include <stdexcept>
#include <string>

namespace mk {
namespace swig {

/*
    Design/implementation/integration notes:

    1. To use this class with SWIG, `%include "std_string.i"`. We cannot avoid
       using `std::string`, because we need to express the concept that the
       caller takes ownership of the value returned by wait_for_next_event().

    2. Move semantic is not wrapped by SWIG. We replace it with passing a
       constant reference for the settings of the Task.

    3. Parsing of the stringified JSON can throw. We have therefore created a
       specific method for initializing, given that throwing in general is
       something that makes life more complicate for bindings. (We may still
       fail and abort() in case of bad error conditions, but a parse error
       is clearly an avoidable error.)

    4. This class is implemented in terms of the FFI API by design choice, so
       in specific cases MK can export just the ANSI-C API.

    Example minimal SWIG interface file:

    ```
    %module measurement_kit
    %include "std_string.i"
    %{
    #include <measurement_kit/swig.hpp>
    %}
    %include <measurement_kit/swig.hpp>
    ```
*/

/// InitializeExResult is the result returned by Task::initialize_ex().
class InitializeExResult {
  public:
    /// reason is the error that occurred.
    std::string reason;
    /// result tells you whether the call succeeded.
    bool result = false;
};

/// Task is something that Measurement Kit can do.
class Task {
  public:

// Prevent SWIG from attempting to generate wrappers for these.
#ifndef SWIG
    class TaskDeleter {
      public:
        void operator()(mk_task_t *p) {
            if (p != nullptr) {
                mk_task_destroy(p);
            }
        }
    };
    class EventDeleter {
      public:
        void operator()(mk_event_t *p) {
            if (p != nullptr) {
                mk_event_destroy(p);
            }
        }
    };
#endif

    /// Task() creates an empty task.
    Task() {}

    /// initialize() initializes a Task with @p settings. @param settings is
    /// a serialized JSON containing the settings. @return true if the JSON is
    /// valid. @return false on parse error. @return false if called more
    /// than once: this is not expected usage. @remark This method isn't thread
    /// safe, meaning that multiple threads should not try to initialize
    /// this class concurrently; that will probably end up badly.
    bool initialize(const std::string &settings) {
        return initialize_ex(settings).result;
    }

    /// initialize_ex() is like initialize() but returns a more structured
    /// result that allows to see the error that occurred.
    InitializeExResult initialize_ex(const std::string &settings) {
        InitializeExResult rv;
        if (pimpl_ != nullptr) {
            rv.reason = "already initialized";
            return rv;
        }
        mk_task_t *task = nullptr;
        mk_task_error_t err = mk_task_start_ex(&task, settings.c_str());
        pimpl_.reset(task);
        switch (err) {
        case MK_TASK_ENONE:
            rv.result = true;
            break;
        case MK_TASK_EPARSE:
            rv.reason = "parse error";
            break;
        case MK_TASK_EGENERIC:
        default:
            rv.reason = "generic error";
            break;
        }
        return rv;
    }

    /// wait_for_next_event() waits for next event. @return next event as a
    /// serialized JSON. When the task is done, the returned JSON is a `null`
    /// JSON (serialized as "null"). @remark Thread safe, but it would not
    /// make much sense to have multiple reader threads.
    std::string wait_for_next_event() {
        std::unique_ptr<mk_event_t, EventDeleter> evp;
        evp.reset(mk_task_wait_for_next_event(pimpl_.get())); // handles null
        if (evp == nullptr) {
            throw std::runtime_error("null_pointer");
        }
        const char *str = mk_event_serialize(evp.get());
        if (str == nullptr) {
            throw std::runtime_error("null_pointer");
        }
        // Performance consideration: this implementation is that we make two
        // copies of strings. While this may be bad in general, we are dealing
        // with generally small, not-so-frequent strings. We probably should
        // not optimize, therefore. But, if we ever want to do so, we can have
        // a function in the FFI API that takes as second argument a std::string
        // pointer and moves the internal serialization to such pointer. We can
        // not return a std::string in a C API, but it seems that we can get
        // away with pointers to expose some more speed to C++.
        return std::string{str};
    }

    /// is_done() returns true if the task is done. A task is done when its
    /// thread is done and there are no unread events in the events queue
    /// drained by wait_for_next_event(). @remark This method is thread safe.
    bool is_done() const {
        return mk_task_is_done(pimpl_.get()); // handles null
    }

    /// interrupt() interrupts the task ASAP. @remark this method is not
    /// blocking and will just inform the task that it should stop. @remark
    /// this method is thread safe and idempotent.
    void interrupt() {
        mk_task_interrupt(pimpl_.get()); // handles null
    }

    /// ~Task() waits for the task to terminate and then reclaims resources.
    ~Task() {}

  private:
    std::unique_ptr<mk_task_t, TaskDeleter> pimpl_;
};

} // namespace swig
} // namespace mk
#endif // MEASUREMENT_KIT_SWIG_HPP
