// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/engine/task.hpp"

#include <assert.h>

#include <atomic>
#include <condition_variable>
#include <deque>
#include <future>
#include <memory>
#include <mutex>
#include <thread>
#include <type_traits>
#include <utility>

#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/nlohmann/json.hpp>
#include <measurement_kit/common/shared_ptr.hpp>

#include "src/libmeasurement_kit/common/worker.hpp"
#include "src/libmeasurement_kit/engine/autoapi.hpp"

namespace mk {
namespace engine {

void Task::emit(nlohmann::json event) {
    // Perform validation of the event (debug mode only)
    event = possibly_validate_event(std::move(event));
    // Actually emit the event.
    {
        std::unique_lock<std::mutex> _{pimpl_->mutex};
        pimpl_->deque.push_back(std::move(event));
    }
    // More efficient if unlocked. Note that we assume that the user
    // could use more than a single thread to drain the queue.
    pimpl_->cond.notify_all();
}

Task::Task(nlohmann::json &&settings) {
    pimpl_ = std::make_unique<TaskImpl>();
    {
        nlohmann::json event;
        event["key"] = "status.queued";
        event["value"] = nlohmann::json::object();
        emit(std::move(event));
    }
    pimpl_->running = true;
    Worker::default_tasks_queue()->call_in_thread(
            Logger::make(), [this, settings = std::move(settings)]() mutable {
                task_run_legacy(this, pimpl_.get(), std::move(settings));
                pimpl_->running = false;
                pimpl_->cond.notify_all(); // tell the readers we're done
            });
}

bool Task::is_done() const {
    std::unique_lock<std::mutex> lock{pimpl_->mutex};
    // Rationale: when the task thread terminates, there may be some
    // unread events in queue. We do not consider the task as done until
    // such events have been read and processed by our controller.
    return pimpl_->running == false && pimpl_->deque.empty();
}

void Task::interrupt() {
    // both variables are safe to use in a MT context
    pimpl_->reactor->stop();
    pimpl_->interrupted = true;
}

nlohmann::json Task::wait_for_next_event() {
    std::unique_lock<std::mutex> lock{pimpl_->mutex};
    // purpose: block here until we stop running or we have events to read
    pimpl_->cond.wait(lock, [this]() { //
        return !pimpl_->running || !pimpl_->deque.empty();
    });
    // must be first so we drain the queue before emitting the final
    // "task_terminated" event.
    if (!pimpl_->deque.empty()) {
        auto rv = std::move(pimpl_->deque.front());
        pimpl_->deque.pop_front();
        return rv;
    }
    assert(!pimpl_->running);
    // Rationale: we used to return `null` when done. But then I figured that
    // this could break people code, because they need to write conditional
    // coding for handling both an ordinary event and `null`. So, to ease the
    // integrator's life, we now return a dummy, well formed event.
    return possibly_validate_event(nlohmann::json{
        {"key", "task_terminated"},
        {"value", nlohmann::json::object()},
    });
}

Task::~Task() {}

} // namespace engine
} // namespace mk
