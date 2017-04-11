// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/nettests.hpp>

#include <atomic>
#include <cassert>
#include <future>
#include <thread>

namespace mk {
namespace nettests {

struct RunnerCtx {
    std::atomic<int> active{0};
    Var<Reactor> reactor = Reactor::global();
    std::atomic<bool> running{false};
    std::thread thread;
};

Runner::Runner() { ctx_.reset(new RunnerCtx); }

void Runner::start_test(Var<Runnable> test, Callback<Var<Runnable>> fn) {
    // Note: here we MUST force the runnable's reactor to be our reactor
    // otherwise we cannot run the specified test...
    assert(not test->reactor);
    test->reactor = ctx_->reactor;
    assert(ctx_->active >= 0);
    if (not ctx_->running) {
        std::promise<bool> promise;
        std::future<bool> future = promise.get_future();
        // WARNING: below we're passing `this` to the thread, which means that
        // the destructor MUST wait the thread. Otherwise, when the thread dies
        // many strange things could happen (I have seen SIGABRT).
        debug("runner: starting reactor in background...");
        ctx_->thread = std::thread([&promise, this]() {
            ctx_->reactor->loop_with_initial_event([&promise]() {
                debug("runner: starting reactor in background... ok");
                promise.set_value(true);
            });
        });
        future.wait();
        ctx_->running = true;
    }
    ctx_->active += 1;
    debug("runner: scheduling %p", (void *)test.get());
    ctx_->reactor->call_soon([=]() {
        debug("runner: starting %p", (void *)test.get());
        test->begin([=](Error) {
            // TODO: do not ignore the error
            debug("runner: ending %p", (void *)test.get());
            test->end([=](Error) {
                // TODO: do not ignore the error
                debug("runner: cleaning-up %p", (void *)test.get());
                // For robustness, delay the final callback to the beginning of
                // next I/O cycle to prevent possible user after frees. This
                // could happen because, in our current position on the stack,
                // we have been called by `Runnable` code that may use `this`
                // after calling the callback. But this would be a problem
                // because `test` is most likely to be destroyed after `fn()`
                // returns. This, when unwinding the stack, the use after free
                // would happen.
                ctx_->reactor->call_soon([=]() {
                    debug("runner: callbacking %p", (void *)test.get());
                    ctx_->active -= 1;
                    debug("runner: #active tasks: %d", (int)ctx_->active);
                    fn(test);
                });
            });
        });
    });
}

void Runner::break_loop_() { ctx_->reactor->break_loop(); }

bool Runner::empty() { return ctx_->active == 0; }

void Runner::join_() {
    if (ctx_->running) {
        ctx_->thread.join();
        ctx_->running = false;
    }
}

Runner::~Runner() {
    // WARNING: This MUST be here to make sure we break the loop before we
    // stop the thread. Not doing that leads to undefined behavior.
    break_loop_();
    join_();
}

/*static*/ Var<Runner> Runner::global() {
    static Var<Runner> singleton(new Runner);
    return singleton;
}

} // namespace nettests
} // namespace mk
