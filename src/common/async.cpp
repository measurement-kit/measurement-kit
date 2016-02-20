// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <event2/thread.h>
#include <atomic>
#include <measurement_kit/common/async.hpp>
#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/net_test.hpp>
#include <measurement_kit/common/poller.hpp>
#include <measurement_kit/common/var.hpp>
#include <thread>

namespace mk {

// Shared state between foreground and background threads
struct AsyncState {
    std::atomic<int> running;
    std::thread thread;
    Poller *poller = Poller::global();
};

class EvThreadSingleton {
  private:
    EvThreadSingleton() { evthread_use_pthreads(); }

  public:
    static void ensure() { static EvThreadSingleton singleton; }
};

void Async::loop_thread(Var<AsyncState> state) {
    EvThreadSingleton::ensure();
    debug("async: loop thread entered");
    state->poller->loop();
    debug("async: exiting from thread");
}

Async::Async(Poller *poller) {
    state.reset(new AsyncState());
    state->poller = poller;
    state->thread = std::thread(loop_thread, state);
}

void Async::run_test(Var<NetTest> test, std::function<void(Var<NetTest>)> fn) {
    debug("async: test inserted: %lld", test->identifier());
    auto context = state;
    ++context->running;
    context->poller->call_soon([context, fn, test]() {
        debug("async: test started: %lld", test->identifier());
        test->begin([context, fn, test]() {
            test->end([context, fn, test]() {
                debug("async: test complete: %lld", test->identifier());
                context->poller->call_soon([context, fn, test]() {
                    debug("async: test removed: %lld", test->identifier());
                    --context->running;
                    fn(test);
                });
            });
        });
    });
}

void Async::break_loop() { state->poller->break_loop(); }

bool Async::empty() { return state->running == 0; }

Async *Async::global() {
    static Async singleton;
    return &singleton;
}

} // namespace mk
