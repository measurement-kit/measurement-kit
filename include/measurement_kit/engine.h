/* Part of Measurement Kit <https://measurement-kit.github.io/>.
   Measurement Kit is free software under the BSD license. See AUTHORS
   and LICENSE for more information on the copying conditions. */
#ifndef MEASUREMENT_KIT_ENGINE_H
#define MEASUREMENT_KIT_ENGINE_H

#ifdef __cplusplus
#ifdef MK_ENGINE_INTERNALS

#include <memory>

#include <measurement_kit/common/nlohmann/json.hpp>

namespace mk {
namespace engine {

/// TaskImpl is the opaque implementation of Task.
class TaskImpl;

/// Task is a task that Measurement Kit can run. You create a task with Task()
/// by passing it the desired settings as a nlohmann::json. The minimal settings
/// JSON must include the task type (see MK_ENUM_TASK for all types).
///
/// Creating a Task also creates the thread that will run it. Altough you can
/// construct more than one Task at a time, Measurement Kit will make sure that
/// tasks run sequentially in the order in which they were created.
///
/// If you enabled task events (see MK_ENUM_EVENT), a Task will emit
/// events while running. You can retrieve events using wait_for_next_event(),
/// which will block until the next event occurs. Regardless of whether you
/// did enable any event, wait_for_next_event() will return a `null` JSON
/// event when the task has terminated.
///
/// You can also interrupt a running task using interrupt().
///
/// The destructor, ~Task(), will join on the Task thread. That is, it will
/// wait for the Task to complete before destroying all the resources.
///
/// See example/engine/ndt.cpp for an example of usage.
class Task {
  public:
    /// Task() creates and starts a Task configured according to @p settings.
    explicit Task(nlohmann::json &&settings);

    /// wait_for_next_event() blocks until the next event occurs.
    nlohmann::json wait_for_next_event();

    /// interrupt() forces the Task to stop running ASAP.
    void interrupt();

    /// ~Task() waits for the task to finish and deallocates resources.
    ~Task();

  private:
    std::unique_ptr<TaskImpl> pimpl_;
};

} // namespace engine
} // namespace mk
#endif // MK_ENGINE_INTERNALS
#endif /* __cplusplus */

/**
 * MK_ENUM_VERBOSITY enumerates all the possible verbosity values. To
 * specify the verbosity level of a task, you should pass one of this values as
 * string (e.g.  "INFO") when you configure the task verbosity. For example, the
 * minimal JSON to run NDT with verbosity level equal to INFO is:
 *
 * ```JSON
 * {
 * "type": "Ndt",
 * "verbosity": "INFO"
 * }
 * ```
 */
#define MK_ENUM_VERBOSITY(XX)                                           \
    XX(QUIET)                                                                  \
    XX(WARNING)                                                                \
    XX(INFO)                                                                   \
    XX(DEBUG)                                                                  \
    XX(DEBUG2)

/**
 * MK_ENUM_EVENT enumerates all possible event types. To specify the
 * events you want to receive from a task, you should pass the desired event
 * types as strings (e.g. "LOG") when you configure the task's enabled events.
 * For example, the minimal JSON to run NDT with the "LOG" and "PERFORMANCE"
 * events enabled is:
 *
 * ```JSON
 * {
 *   "enabled_events": ["LOG", "PERFORMANCE"],
 *   "type": "Ndt"
 * }
 * ```
 */
#define MK_ENUM_EVENT(XX)                                               \
    XX(LOG)                                                                    \
    XX(PROGRESS)                                                               \
    XX(FAILURE)                                                                \
    XX(PERFORMANCE)                                                            \
    XX(RESULT)

/**
 * MK_ENUM_TASK enumerates the task that Measurement Kit can run. When
 * you want to run a task, you must specify the task type as a string. For
 * example, the minimal JSON to run NDT is:
 *
 * ```JSON
 *   {"type": "Ndt"}
 * ```
 */
#define MK_ENUM_TASK(XX)                                                \
    XX(Dash)                                                                   \
    XX(CaptivePortal)                                                          \
    XX(DnsInjection)                                                           \
    XX(FacebookMessenger)                                                      \
    XX(HttpHeaderFieldManipulation)                                            \
    XX(HttpInvalidRequestLine)                                                 \
    XX(MeekFrontedRequests)                                                    \
    XX(MultiNdt)                                                               \
    XX(Ndt)                                                                    \
    XX(TcpConnect)                                                             \
    XX(Telegram)                                                               \
    XX(WebConnectivity)                                                        \
    XX(Whatsapp)

#endif /* MEASUREMENT_KIT_ENGINE_H */
