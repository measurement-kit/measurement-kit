// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/common/async.hpp>
#include <measurement_kit/common/logger.hpp>

#include <mutex>
#include <map>
#include <thread>

#include <event2/thread.h>

//
// TODO: modify this code to allow the user to specify a custom
// poller (commented out code for this already exists)
//

namespace measurement_kit {
namespace common {

typedef SharedPointer<NetTest> NetTestVar;

// Shared state between foreground and background threads
struct AsyncState {
    std::map<NetTest *, NetTestVar> ready;
    std::map<NetTest *, std::function<void(NetTestVar)>> callbacks;
    std::map<NetTest *, NetTestVar> active;
    std::map<NetTest *, NetTestVar> completed;
    volatile bool changed = false;
    std::function<void()> hook_empty;
    volatile bool interrupted = false;
    std::mutex mutex;
    SharedPointer<Poller> poller;
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
#define LOCKED(foo) { \
        std::lock_guard<std::mutex> lck(state->mutex); \
        foo \
    }

// Ensure consistency
#define INSERT(m, k, v) do { \
        auto ret = m.insert(std::make_pair(k, v)); \
        if (!ret.second) throw std::runtime_error("Element already there"); \
    } while (0)
#define GET(v, m, k) do { \
        auto there = (m.find(k) != m.end()); \
        if (!there) throw std::runtime_error("Element not there"); \
        v = m[k]; \
    } while (0)

void Async::loop_thread(SharedPointer<AsyncState> state) {

    EvThreadSingleton::ensure();

    debug("async: loop thread entered");
    for (;;) {

        LOCKED(
            debug("async: loop thread locked");
            debug("async: size of ready: %lu", state->ready.size());
            debug("async: size of callbacks: %lu", state->callbacks.size());
            debug("async: size of active: %lu", state->active.size());
            debug("async: size of completed: %lu", state->completed.size());
            if (state->interrupted) {
                debug("async: interrupted");
                break;
            }
            if (state->ready.empty() && state->active.empty()) {
                debug("async: empty");
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
                        // Safe to lock because loop_once() is unlocked
                        LOCKED(
                            debug("async: need to lock to move test object");
                            NetTestVar var;
                            GET(var, state->active, ptr);
                            INSERT(state->completed, ptr, var);
                            state->active.erase(ptr);
                            state->changed = true;
                            debug("async: completed: %lld", var->identifier());
                            debug("async: done with briefly locking");
                        )
                    });
                });
            }
            state->ready.clear();
        )

        debug("async: loop thread unlocked");
        while (!state->changed) {
            //state->poller->loop_once();
            loop_once();
        }
        state->changed = false;
        debug("async: bottom of loop thread");
    }

    // TODO: wondering whether the following code should execute
    // locked in the place where we decide we need to break
    debug("async: detaching thread...");
    state->thread.detach();
    state->thread_running = false;
    if (state->hook_empty) {
        state->hook_empty();
    }
    debug("async: exiting from thread");
}

Async::Async(SharedPointer<Poller> poller) {
    state.reset(new AsyncState());
    state->poller = poller;
}

void Async::run_test(NetTestVar test, std::function<void(NetTestVar)> fn) {
    LOCKED(
        NetTest *ptr = test.operator->(); // get() does not check for NULL
        INSERT(state->ready, ptr, test);
        INSERT(state->callbacks, ptr, fn);
        state->changed = true;
        debug("async: ready: %lld", test->identifier());
        if (!state->thread_running) {
            debug("async: background thread started");
            state->thread = std::thread(loop_thread, state);
            state->thread_running = true;
        }
    )
}

void Async::break_loop() {
    //state->poller->break_loop();  // Idempotent
    break_loop();
    state->interrupted = true;
}

// TODO: make sure this is enough to restart loop
void Async::restart_loop() {
    state->interrupted = false;
}

bool Async::empty() {
    return !state->thread_running;
}

void Async::on_empty(std::function<void()> fn) {
    LOCKED(
        state->hook_empty = fn;
    )
}

void Async::pump() {
    std::map<NetTest *, NetTestVar> context;
    std::map<NetTest *, std::function<void(NetTestVar)>> funcs;
    LOCKED(
        for (auto pair : state->completed) {
            INSERT(context, pair.first, pair.second);
            std::function<void(NetTestVar)> func;
            GET(func, state->callbacks, pair.first);
            INSERT(funcs, pair.first, func);
            state->callbacks.erase(pair.first);
            debug("async: zombie: %lld", pair.first->identifier());
        }
        state->completed.clear();
        state->changed = true;
    )
    for (auto pair : funcs) {
        NetTestVar var;
        GET(var, context, pair.first);
        pair.second(var);
    }
}

} // namespace common
} // namespace measurement_kit
