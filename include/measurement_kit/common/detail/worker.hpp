// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_DETAIL_WORKER_HPP
#define MEASUREMENT_KIT_COMMON_DETAIL_WORKER_HPP

#include <measurement_kit/common/callback.hpp>
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/non_copyable.hpp>
#include <measurement_kit/common/non_movable.hpp>
#include <measurement_kit/common/var.hpp>

#include <chrono>
#include <cassert>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>

namespace mk {

class Worker {
  public:
    class State : public NonCopyable, public NonMovable {
      public:
        unsigned short active = 0;
        std::mutex mutex;
        unsigned short parallelism = 3;
        std::list<Callback<>> queue;
    };

    void run_in_background_thread(Callback<> &&func) {
        std::unique_lock<std::mutex> _{state->mutex};

        // Move function such that the running-in-background thread
        // has unique ownership and controls its lifecycle.
        state->queue.push_back(std::move(func));

        if (state->active >= state->parallelism) {
            return;
        }

        // Note: pass only the internal state, so that the thread can possibly
        // continue to work even when the external object is gone.
        auto task = [S = state]() {
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
                try {
                    func();
                } catch (...) {
                    mk::warn("worker thread: unhandled exception");
                }
            }
        };

        std::thread{task}.detach();
        ++state->active;
    }

    unsigned short parallelism() const {
        std::unique_lock<std::mutex> _{state->mutex};
        return state->parallelism;
    }

    void set_parallelism(unsigned short newval) const {
        std::unique_lock<std::mutex> _{state->mutex};
        state->parallelism = newval;
    }

    unsigned short concurrency() const {
        std::unique_lock<std::mutex> _{state->mutex};
        return state->active;
    }

    void wait_empty_() const {
        // Implementation note: this method is meant to be used in regress
        // tests, where we don't want the test to exit until the background
        // thread has exited, so to clear thread-local storage. Othrwise,
        // Valgrind will complain about leaked thread-local storage.
        //
        // We expect the caller to issue a blocking command using a Worker
        // and then to call this method such that we keep the main thread
        // alive for longer, so that background threads can exit.
        //
        // Since this is meant for internal-only usage, as explained above,
        // it has been given a name terminating with `_`.
        //
        // See:
        // - test/ooni/orchestrate.cpp
        // - test/nettests/utils.hpp
        //
        while (concurrency() > 0) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    static Var<Worker> default_tasks_queue() {
        static Var<Worker> worker = Var<Worker>::make();
        return worker;
    }

  private:
    std::shared_ptr<State> state{std::make_shared<State>()};
};

} // namespace mk
#endif
