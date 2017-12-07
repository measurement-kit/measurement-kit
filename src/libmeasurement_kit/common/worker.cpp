// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include <measurement_kit/common/callback.hpp>
#include "private/common/worker.hpp"
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/non_copyable.hpp>
#include <measurement_kit/common/non_movable.hpp>
#include <measurement_kit/common/shared_ptr.hpp>

#include <cassert>
#include <chrono>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>

namespace mk {

Worker::Worker() {}

Worker::Worker(size_t p) { state->parallelism = p; }

void Worker::call_in_thread(SharedPtr<Logger> logger, Callback<> &&func) {
    std::unique_lock<std::mutex> _{state->mutex};

    // Move function such that the running-in-background thread
    // has unique ownership and controls its lifecycle.
    state->queue.push_back(std::move(func));

    if (state->active >= state->parallelism) {
        return;
    }

    // Note: pass only the internal state, so that the thread can possibly
    // continue to work even when the external object is gone.
    auto task = [S = state, logger]() {
        for (;;) {
            Callback<> func = [&]() {
                std::unique_lock<std::mutex> _{S->mutex};
                // Initialize inside the lock such that there is only
                // one critical section in which we could be
                if (S->queue.size() <= 0) {
                    --S->active;
                    return Callback<>{};
                }
                auto front = S->queue.front();
                S->queue.pop_front();
                return front;
            }();
            if (!func) {
                break;
            }
            // Exceptions are fatal in measurement-kit. If we get an unhandled
            // one here is a bug that must be fixed. Make sure it is logged
            // using the current logger and bail.
            try {
                func();
            } catch (const std::exception &exc) {
                logger->warn("worker: unhandled exception: %s", exc.what());
                std::rethrow_exception(std::current_exception());
            } catch (...) {
                logger->warn("worker: unhandled unknown exception");
                std::rethrow_exception(std::current_exception());
            }
        }
    };

    std::thread{task}.detach();
    ++state->active;
}

unsigned short Worker::parallelism() const {
    std::unique_lock<std::mutex> _{state->mutex};
    return state->parallelism;
}

void Worker::set_parallelism(unsigned short newval) const {
    std::unique_lock<std::mutex> _{state->mutex};
    state->parallelism = newval;
}

unsigned short Worker::concurrency() const {
    std::unique_lock<std::mutex> _{state->mutex};
    return state->active;
}

void Worker::wait_empty_() const {
    while (concurrency() > 0) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

/*static*/ SharedPtr<Worker> Worker::default_tasks_queue() {
    static SharedPtr<Worker> worker{std::make_shared<Worker>(1)};
    return worker;
}

} // namespace mk
