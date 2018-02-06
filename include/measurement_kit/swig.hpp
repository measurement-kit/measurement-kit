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

#include <memory>
#include <string>

namespace mk {

namespace engine {
class Task; // forward decl.
} // namespace engine

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
       is clearly an avoidable error.

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

/// Task wraps mk::engine::Task for SWIG consumption.
class Task {
  public:
    /// Task() creates an empty task.
    Task();

    /// initialize() initializes a Task with @p settings. @param settings is
    /// a serialized JSON containing the settings. @return true if the JSON is
    /// valid, false on parse error. @return false if called more than once:
    /// this class is not designed for being initialized more than once. @see
    /// mk::engine::Task for more information on the structure of the JSON
    /// that can be passed as the @p settings parameter. @remark This method
    /// is not thread safe, meaning that multiple threads should not try to
    /// initialize this class concurrently.
    bool initialize(const std::string &settings);

    /// wait_for_next_event() waits for next event. @return next event as a
    /// serialized JSON. When the task is done, the returned JSON is a `null`
    /// JSON (serialized as "null"). @see mk::engine::Task for more information
    /// about the structure of the JSON returned by each event. @remark using
    /// multiple threads for reading events using this method will not crash
    /// the code but will also not make any sense.
    std::string wait_for_next_event();

    /// interrupt() interrupts the task ASAP. @remark this method is not
    /// blocking and will just inform the task that it should stop. @remark
    /// this method is thread safe and idempotent.
    void interrupt();

    /// ~Task() waits for the task to terminate and then reclaims resources.
    ~Task();

  private:
    std::unique_ptr<engine::Task> pimpl_;
};

} // namespace swig
} // namespace mk
#endif // MEASUREMENT_KIT_SWIG_HPP
