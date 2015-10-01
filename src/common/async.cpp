// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/common/async.hpp>
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/net_test.hpp>
#include <measurement_kit/common/poller.hpp>
#include <measurement_kit/common/pointer.hpp>

#include <mutex>
#include <map>
#include <stdexcept>
#include <thread>
#include <utility>

#include <event2/thread.h>

namespace measurement_kit {
namespace common {

typedef SharedPointer<NetTest> NetTestVar;

// Shared state between foreground and background threads
struct AsyncState {
    std::map<NetTest *, NetTestVar> ready;
    std::map<NetTest *, std::function<void(NetTestVar)>> done_callbacks;
    std::map<NetTest *, std::function<void(
            std::list<std::string> &)>> log_callbacks;
    std::map<NetTest *, NetTestVar> active;
    std::map<NetTest *, NetTestVar> completed;
    std::map<NetTest *, std::list<std::string>> logs;
    volatile bool changed = false;
    volatile bool interrupted = false;
    std::mutex mutex_gen;
    std::mutex mutex_logs;
    std::thread thread;
    volatile bool thread_running = false;
};

class EvThreadSingleton {
  private:
    EvThreadSingleton() { evthread_use_pthreads(); }

  public:
    static void ensure() { static EvThreadSingleton singleton; }
};

// Syntactic sugar
#define LOCKED(foo)                                                            \
    {                                                                          \
        std::lock_guard<std::mutex> lck(state->mutex_gen);                     \
        foo                                                                    \
    }

// Syntactic sugar
#define LOCKED_LOGS(foo)                                                       \
    {                                                                          \
        std::lock_guard<std::mutex> lck(state->mutex_logs);                    \
        foo                                                                    \
    }

// Ensure consistency
#define INSERT(m, k, v)                                                        \
    do {                                                                       \
        auto ret = m.insert(std::make_pair(k, v));                             \
        if (!ret.second) throw std::runtime_error("Element already there");    \
    } while (0)
#define GET(v, m, k)                                                           \
    do {                                                                       \
        auto there = (m.find(k) != m.end());                                   \
        if (!there) throw std::runtime_error("Element not there");             \
        v = m[k];                                                              \
    } while (0)

void Async::loop_thread(SharedPointer<AsyncState> state) {

    EvThreadSingleton::ensure();

    state->interrupted = false; // Undo previous break_loop() if any

    debug("async: loop thread entered");
    for (;;) {

        LOCKED({
            debug("async: loop thread locked");
            debug("async: size of ready: %lu", state->ready.size());
            debug("async: size of done_callbacks: %lu",
                    state->done_callbacks.size());
            debug("async: size of log_callbacks: %lu",
                    state->log_callbacks.size());
            debug("async: size of active: %lu", state->active.size());
            debug("async: size of completed: %lu", state->completed.size());
            debug("async: size of logs: %lu", state->logs.size());
            auto empty = (state->ready.empty() && state->active.empty());
            if (state->interrupted || empty) {
                debug("async: %s", (empty) ? "empty" : "interrupted");
                // Do this here so it's done while we're locked
                debug("async: detaching thread...");
                state->thread.detach();
                state->thread_running = false;
                break;
            }
            debug("async: not interrupted and not empty");
            for (auto pair : state->ready) {
                NetTest *ptr = pair.first;
                debug("async: active: %lld", ptr->identifier());
                INSERT(state->active, ptr, pair.second);
                // Note: it should be safe to capture `state` by reference
                // here because the destructor should not be able to destroy
                // the background thread until we detach it and break.
                ptr->on_log([&state, ptr](const char *log_line) {
                    // We use a second mutex to protect logs because we
                    // would otherwise deadlock on the first mutex if the
                    // test immediately generates logs
                    LOCKED_LOGS({ state->logs[ptr].push_back(log_line); })
                });
                ptr->begin([&state, ptr]() {
                    ptr->end([&state, ptr]() {
                        // Safe to lock because loop_once() is unlocked
                        LOCKED({
                            debug("async: need to lock to move test object");
                            NetTestVar var;
                            GET(var, state->active, ptr);
                            INSERT(state->completed, ptr, var);
                            state->active.erase(ptr);
                            state->changed = true;
                            debug("async: completed: %lld", var->identifier());
                            debug("async: done with briefly locking");
                        })
                    });
                });
            }
            state->ready.clear();
        })

        debug("async: loop thread unlocked");
        state->changed = false;
        while (!state->changed) {
            loop_once();
        }
        debug("async: bottom of loop thread");
    }
    debug("async: exiting from thread");
}

Async::Async() { state.reset(new AsyncState()); }

void Async::run_test(NetTestVar test,
        std::function<void(std::list<std::string> &)> log_fn,
        std::function<void(NetTestVar)> done_fn) {
    LOCKED({
        NetTest *ptr = test.get();
        INSERT(state->ready, ptr, test);
        INSERT(state->done_callbacks, ptr, done_fn);
        INSERT(state->log_callbacks, ptr, log_fn);
        state->changed = true;
        debug("async: ready: %lld", test->identifier());
        if (!state->thread_running) {
            debug("async: background thread started");
            state->thread = std::thread(loop_thread, state);
            state->thread_running = true;
        }
    })
}

void Async::break_loop() {
    break_loop();
    state->interrupted = true;
    state->changed = true;
}

bool Async::empty() { return !state->thread_running; }

void Async::pump() {
    std::map<NetTest *, NetTestVar> context;
    std::map<NetTest *, std::function<void(NetTestVar)>> funcs;
    LOCKED({
        // We lock both the general and the logs mutex because both the
        // logs and the log_callbacks objects must be stable
        LOCKED_LOGS({
            for (auto pair : state->logs) {
                std::function<void(std::list<std::string> &)> fn;
                debug("async: process logs of %lld...",
                        pair.first->identifier());
                GET(fn, state->log_callbacks, pair.first);
                fn(pair.second);
            }
            state->logs.clear();
        })
        if (state->completed.empty()) return; /* shortcut */
        for (auto pair : state->completed) {
            INSERT(context, pair.first, pair.second);
            std::function<void(NetTestVar)> func;
            GET(func, state->done_callbacks, pair.first);
            INSERT(funcs, pair.first, func);
            state->done_callbacks.erase(pair.first);
            state->log_callbacks.erase(pair.first);
            state->logs.erase(pair.first);
            debug("async: zombie: %lld", pair.first->identifier());
        }
        state->completed.clear();
        state->changed = true;
    })
    for (auto pair : funcs) {
        NetTestVar var;
        GET(var, context, pair.first);
        pair.second(var);
    }
}

} // namespace common
} // namespace measurement_kit
