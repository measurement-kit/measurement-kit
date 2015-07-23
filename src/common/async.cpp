// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/common/async.hpp>
#include <measurement_kit/common/log.hpp>

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

// Shared state between foreground and background threads
struct AsyncState {
    std::map<SharedPointer<NetTest>,
      std::function<void(SharedPointer<NetTest>)>> active;
    volatile bool changed = false;
    std::function<void()> hook_empty;
    volatile bool interrupted = false;
    std::mutex mutex;
    SharedPointer<Poller> poller;
    std::map<SharedPointer<NetTest>,
      std::function<void(SharedPointer<NetTest>)>> ready;
    std::thread thread;
    volatile bool thread_running = false;
};

class EvThreadSingleton {

  private:
    EvThreadSingleton() {
        evthread_use_pthreads();
    }

  public:
    static void ensure() {
        static EvThreadSingleton singleton;
    }

};

// Syntactic sugar
#define LOCKED(foo) { \
        std::lock_guard<std::mutex> lck(state->mutex); \
        foo \
    }

void Async::loop_thread(SharedPointer<AsyncState> state) {

    EvThreadSingleton::ensure();

    debug("async: loop thread entered");
    for (;;) {

        LOCKED(
            debug("async: loop thread locked");
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
                state->active.insert(pair);
                pair.first->begin([state, pair]() {
                    pair.first->end([state, pair]() {
                        //
                        // This callback is invoked by loop_once, when we do
                        // not own the lock. For this reason it's important
                        // to only modify state->active in the current thread,
                        // i.e. in the background thread (i.e this function)
                        //
                        state->active.erase(pair.first);
                        state->changed = true;
                        debug("async: test stopped");
                        if (pair.second) {
                            pair.second(pair.first);
                        }
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
    debug("async: thread detached");
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

void Async::run_test(SharedPointer<NetTest> test,
  std::function<void(SharedPointer<NetTest>)> fn) {
    LOCKED(
        debug("async: test inserted");
        auto ret = state->ready.insert(std::make_pair(test, fn));
        if (!ret.second) {
            throw std::runtime_error("async: element already inserted");
        }
        state->changed = true;
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

}}
