/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#include <ight/common/async.hpp>
#include <ight/common/log.hpp>

#include <mutex>
#include <set>
#include <thread>

using namespace ight::common::async;
using namespace ight::common::pointer;

namespace ight {
namespace common {
namespace async {

// Shared state between foreground and background threads
struct AsyncState {
    std::set<SharedPointer<NetTest>> active;
    volatile bool changed = false;
    volatile bool interrupted = false;
    std::mutex mutex;
    SharedPointer<Poller> poller;
    std::set<SharedPointer<NetTest>> ready;
    std::thread thread;
    volatile bool thread_running = false;
};

}}}

// Syntactic sugar
#define LOCKED(foo) { \
        std::lock_guard<std::mutex> lck(state->mutex); \
        foo \
    }

void Async::loop_thread(SharedPointer<AsyncState> state) {
    ight_debug("loop thread entered");
    for (;;) {

        LOCKED(
            ight_debug("loop thread locked");
            if (state->interrupted) {
                ight_debug("interrupted");
                return;
            }
            if (state->ready.empty() && state->active.empty()) {
                ight_debug("empty");
                return;
            }
            ight_debug("not interrupted and not empty");
            for (auto test : state->ready) {
                state->active.insert(test);
                test->begin([state, test]() {
                    test->end([state, test]() {
                        //
                        // This callback is invoked by loop_once, when we do
                        // not own the lock. For this reason it's important
                        // to only modify state->active in the current thread
                        //
                        state->active.erase(test);
                        state->changed = true;
                        ight_debug("*** test stopped");
                    });
                });
            }
            state->ready.clear();
        )

        ight_debug("loop thread unclocked");

        while (!state->changed) {
            //state->poller->loop_once();
            ight_loop_once();
        }
        state->changed = false;

        ight_debug("bottom of loop thread");
    }
    state->thread_running = false;
    ight_debug("thread stopped");
}

Async::Async(SharedPointer<Poller> poller) {
    state.reset(new AsyncState());
    state->poller = poller;
}

void Async::run_test(SharedPointer<NetTest> test) {
    LOCKED(
        ight_debug("event inserted");
        state->ready.insert(test);
        state->changed = true;
        if (!state->thread_running) {
            ight_debug("thread started");
            state->thread = std::thread(loop_thread, state);
            state->thread_running = true;
        }
    )
}

void Async::break_loop() {
    //state->poller->break_loop();  // Idempotent
    ight_break_loop();
    state->interrupted = true;
}

void Async::restart_loop() {
    state->interrupted = true;
}
