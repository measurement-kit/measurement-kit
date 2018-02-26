/* Part of Measurement Kit <https://measurement-kit.github.io/>.
   Measurement Kit is free software under the BSD license. See AUTHORS
   and LICENSE for more information on the copying conditions. */
#ifndef MEASUREMENT_KIT_ENGINE_H
#define MEASUREMENT_KIT_ENGINE_H

/*
 * Low level engine for running Measurement Kit tests. It's not exposed by
 * default because it depends on the nlohmann/json.hpp used to compile
 * Measurement Kit, which may change from release to release. If you want
 * to depend on this API, you can, just define MK_ENGINE_INTERNALS, and be
 * prepared to deal with ABI changes when we upgrade nlohmann/json.hpp.
 *
 * See example/engine/ndt.cpp for example usage.
 */

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
/// JSON must include the task name (see MK_ENUM_TASK for all names).
///
/// Creating a Task also creates the thread that will run it. Altough you can
/// construct more than one Task at a time, Measurement Kit will make sure that
/// tasks run sequentially in the order in which they were created.
///
/// A Task will emits events while running, which you can retrieve using the
/// wait_for_next_event() call, which blocks until next event occurs. You can
/// configure a Task to disable some or all events. Regardless of whether there
/// are enabled events, wait_for_next_event() will return the `null` JSON
/// when the task has terminated.
///
/// To know whether a task has finished running, use is_done(). The method will
/// return true when the task thread has exited and there are no unread events
/// in the queue drained by wait_for_next_event().
///
/// You can also interrupt a running task using interrupt().
///
/// The destructor, ~Task(), will join on the Task thread. That is, it will
/// wait for the Task to complete before destroying all the resources.
///
/// See example/engine/ndt.cpp for an example of usage.
class Task {
  public:
    /// Task() creates and starts a Task configured according settings. See
    /// the MK_ENUM_SETTINGS macro for more information.
    explicit Task(nlohmann::json &&settings);

    /// wait_for_next_event() blocks until the next event occurs.
    nlohmann::json wait_for_next_event();

    /// is_done() returns true when the task has finished running and there
    /// are no unread events in wait_for_next_event()'s queue.
    bool is_done() const;

    /// interrupt() forces the Task to stop running ASAP.
    void interrupt();

    /// ~Task() waits for the task to finish and deallocates resources.
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
#endif // MK_ENGINE_INTERNALS
#endif /* __cplusplus */

/**
 * MK_ENUM_SETTINGS enumerates the possible keys that the settings object
 * passed to a task should have. The XX macro takes three arguments: the
 * name of the setting, the type that the key should have, and whether the
 * specified setting must be present in the JSON object.
 */
#define MK_ENUM_SETTINGS(XX)                                                   \
    XX(annotations, object, false)                                             \
    XX(disabled_events, array, false)                                          \
    XX(inputs, array, false)                                                   \
    XX(input_files, array, false)                                              \
    XX(log_file, string, false)                                                \
    XX(options, object, false)                                                 \
    XX(output_file, string, false)                                             \
    XX(verbosity, string, false)                                               \
    XX(name, string, true)

/**
 * MK_ENUM_VERBOSITY enumerates all the possible verbosity values. To
 * specify the verbosity level of a task, you should pass one of this values as
 * string (e.g.  "INFO") when you configure the task verbosity. For example, the
 * minimal JSON to run NDT with verbosity level equal to INFO is:
 *
 * ```JSON
 * {
 * "name": "Ndt",
 * "verbosity": "INFO"
 * }
 * ```
 */
#define MK_ENUM_VERBOSITY(XX)                                                  \
    XX(ERR)                                                                    \
    XX(WARNING)                                                                \
    XX(INFO)                                                                   \
    XX(DEBUG)                                                                  \
    XX(DEBUG2)

/**
 * MK_ENUM_EVENT enumerates all possible event keys. By default all events
 * are enabled, but you can disable specific events using the "disabled_events"
 * key of the settings object. For example, to disable "log", use:
 *
 * ```JSON
 * {
 *   "disabled_events": ["log"],
 *   "name": "Ndt"
 * }
 * ```
 */
#define MK_ENUM_EVENT(XX)                                                      \
    XX("failure.measurement")                                                  \
    XX("failure.report_submission")                                            \
    XX("failure.startup")                                                      \
    XX("log")                                                                  \
    XX("measurement_entry")                                                    \
    XX("status.end")                                                           \
    XX("status.geoip_lookup")                                                  \
    XX("status.progress")                                                      \
    XX("status.queued")                                                        \
    XX("status.report_created")                                                \
    XX("status.started")                                                       \
    XX("status.update.performance")                                            \
    XX("status.update.websites")

/**
 * MK_ENUM_TASK enumerates the task that Measurement Kit can run. When
 * you want to run a task, you must specify the task name as a string. For
 * example, the minimal JSON to run NDT is:
 *
 * ```JSON
 *   {"name": "Ndt"}
 * ```
 */
#define MK_ENUM_TASK(XX)                                                       \
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
