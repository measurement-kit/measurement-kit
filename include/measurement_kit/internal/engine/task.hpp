// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef INCLUDE_MEASUREMENT_KIT_INTERNAL_ENGINE_TASK_HPP
#define INCLUDE_MEASUREMENT_KIT_INTERNAL_ENGINE_TASK_HPP

#include <memory>

#include <measurement_kit/common/nlohmann/json.hpp>

namespace mk {
namespace engine {

/// TaskImpl is the opaque implementation of Task.
class TaskImpl;

/// Task is a task that Measurement Kit can run. You create a task with Task()
/// by passing it the desired settings as a nlohmann::json. The minimal settings
/// JSON must include the "name" key indicating the task name.
///
/// Creating a Task also creates the thread that will run it. Altough you can
/// construct more than one Task at a time, Measurement Kit will make sure that
/// tasks do not run concurrently, even though there is no guarantee that they
/// will actually run sequentially.
///
/// A Task will emit events while running, which you can retrieve using the
/// wait_for_next_event() call, which blocks until next event occurs. You can
/// configure a Task to disable some or all events.
///
/// To know whether a task has finished running, use is_done(). The method will
/// return true when the task thread has exited and there are no unread events
/// in the queue drained by wait_for_next_event().
///
/// You can also interrupt a running task using interrupt().
///
/// The destructor, ~Task(), will join on the Task thread. That is, it will
/// wait for the Task to complete before destroying all the resources.
class Task {
  public:
    /// Task creates and starts a Task configured according to settings.
    explicit Task(nlohmann::json &&settings);

    /// wait_for_next_event blocks until the next event occurs. When the
    /// task is terminated, it returns the "task_terminated" dummy event.
    nlohmann::json wait_for_next_event();

    /// emit emits a specific event by appending it to the queue that
    /// is drained by user code via wait_for_next_event.
    void emit(nlohmann::json event);

    /// is_done returns true when the task has finished running and there
    /// are no unread events in wait_for_next_event's queue.
    bool is_done() const;

    /// interrupt forces the Task to stop running ASAP.
    void interrupt();

    /// ~Task waits for the task to finish and deallocates resources.
    ~Task();

    // Implementation note: this class is _explicitly_ non copyable and non
    // movable so we don't have to worry about pimpl's validity.
    Task(const Task &) noexcept = delete;
    Task &operator=(const Task &) noexcept = delete;
    Task(Task &&) noexcept = delete;
    Task &operator=(Task &&) noexcept = delete;

  private:
    std::unique_ptr<TaskImpl> pimpl_;
};

} // namespace engine
} // namespace mk
#endif
