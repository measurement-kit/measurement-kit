// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/common/async.hpp>
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/net_test.hpp>
#include <measurement_kit/common/poller.hpp>

#include <mutex>
#include <map>
#include <thread>

#include <event2/thread.h>

namespace mk {

// Shared state between foreground and background threads
struct AsyncState {
    std::map<NetTest *, std::function<void(Var<NetTest>)>> callbacks;
    std::map<NetTest *, Var<NetTest>> ready;
    std::map<NetTest *, Var<NetTest>> active;
    std::map<NetTest *, Var<NetTest>> complete;
    volatile bool interrupted = false;
    std::mutex mutex;
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
        std::lock_guard<std::mutex> lck(state->mutex);                         \
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

void Async::loop_thread(Var<AsyncState> state) {
    EvThreadSingleton::ensure();
    state->interrupted = false; // Undo previous break_loop() if any

    debug("async: loop thread entered");
    for (;;) {

        LOCKED({
            debug("async: loop thread locked");
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
                ptr->begin([&state, ptr]() {
                    ptr->end([&state, ptr]() {
                        //
                        // This callback is invoked by loop_once, when we do
                        // not own the lock. For this reason it's important
                        // to only modify state->active in the current thread,
                        // i.e. in the background thread (i.e this function)
                        //
                        debug("async: test stopped");
                        Var<NetTest> test;
                        std::function<void(Var<NetTest>)> callback;
                        LOCKED({
                            GET(test, state->active, ptr);
                            GET(callback, state->callbacks, ptr);
                            state->active.erase(ptr);
                            state->callbacks.erase(ptr);
                            // Keep alive test until end of the loop
                            INSERT(state->complete, ptr, test);
                        });
                        if (callback) {
                            callback(test);
                        }
                    });
                });
            }
            state->ready.clear();
        })

        debug("async: loop thread unlocked");
        loop_once();

        LOCKED({
            // Do this here so tests have a chance to rewind their stack
            debug("async: clearing terminated tests");
            state->complete.clear();
        })
        debug("async: bottom of loop thread");
    }
    debug("async: exiting from thread");
}

Async::Async() { state.reset(new AsyncState()); }

void Async::run_test(Var<NetTest> test, std::function<void(Var<NetTest>)> fn) {
    LOCKED({
        debug("async: test inserted");
        INSERT(state->ready, test.get(), test);
        INSERT(state->callbacks, test.get(), fn);
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
}

bool Async::empty() { return !state->thread_running; }

Async *Async::global() {
    static Async singleton;
    return &singleton;
}

} // namespace mk
