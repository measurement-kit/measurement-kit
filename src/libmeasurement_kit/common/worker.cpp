// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "private/common/worker.hpp"
#include <measurement_kit/common/callback.hpp>
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

void Worker::on_error(Callback<std::exception_ptr> &&func) {
    std::unique_lock<std::mutex> _{state->mutex};
    std::swap(func, state->error_cb);
}

void Worker::call_in_thread(Callback<> &&func) {
    std::unique_lock<std::mutex> _{state->mutex};

    // Move function such that the running-in-background thread
    // has unique ownership and controls its lifecycle.
    state->queue.push_back(std::move(func));

    if (state->active >= state->parallelism || state->unhandled_exc) {
        return;
    }

    // Note: pass only the internal state, so that the thread can possibly
    // continue to work even when the external object is gone.
    auto task = [S = state]() {
        for (;;) {
            Callback<> func;
            // To avoid any race we use the following critical section in
            // which we initialize the function to be called.
            {
                std::unique_lock<std::mutex> _{S->mutex};
                if (S->active <= 0) {
                    // This condition cannot happen when we are schedling a new
                    // function task because we increment S->active inside the
                    // protection of the mutex and the new thread will block on
                    // the same mutex just after it has been started.
                    throw std::runtime_error("worker thread: panic");
                } else if (S->unhandled_exc) {
                    // When we have seen one or more exceptions, enter into a
                    // panic state where we do not run tasks anymore and we wait
                    // for the final alive thread to notify the caller that we
                    // experienced some bad issues down here.
                    --S->active;
                    if (S->active <= 0 && S->error_cb) {
                        try {
                            S->error_cb(S->unhandled_exc);
                        } catch (...) {
                            /* SUPPRESS */;
                        }
                    }
                } else if (S->queue.size() <= 0) {
                    --S->active;
                } else {
                    std::swap(func, S->queue.front());
                    S->queue.pop_front();
                }
            }
            if (!func) {
                break;
            }
            std::exception_ptr unhandled_exc;
            try {
                func();
            } catch (const std::exception &exc) {
                mk::warn("worker thread: unhandled exception: %s", exc.what());
                unhandled_exc = std::current_exception();
            } catch (...) {
                mk::warn("worker thread: unhandled exception: <unknown>");
                unhandled_exc = std::current_exception();
            }
            if (unhandled_exc) {
                std::unique_lock<std::mutex> _{S->mutex};
                if (!S->unhandled_exc) {
                    std::swap(S->unhandled_exc, unhandled_exc);
                }
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
    static SharedPtr<Worker> worker = SharedPtr<Worker>::make();
    return worker;
}

} // namespace mk
