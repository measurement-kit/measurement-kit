// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <event2/event.h>
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

static void mk_do_nothing(evutil_socket_t, short, void *p) {}

void Async::loop_thread(Var<AsyncState> state) {
    debug("async: loop thread entered");
    struct timeval ten_seconds = {10, 0};
    // Make sure that, no matter what kind of poller we're using and what kind
    // of
    // events it periodically schedules, if any, there is at least one event
    event *ev = ::event_new(state->poller->get_event_base(), -1, EV_PERSIST,
                            mk_do_nothing, nullptr);
    ::event_add(ev, &ten_seconds);
    state->poller->loop();
    ::event_free(ev);
    debug("async: exiting from thread");
}

Async::Async(Poller *poller) {
    state.reset(new AsyncState());
    state->poller = poller;
    state->thread = std::thread(loop_thread, state);
}

Async::~Async() {
    state->poller->break_loop();
    state->thread.join();
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
