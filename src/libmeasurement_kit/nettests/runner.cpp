// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/nettests.hpp>

#include <atomic>
#include <cassert>

namespace mk {
namespace nettests {

struct RunnerCtx {
    std::atomic<int> active{0};
    std::mutex mutex;
    Var<Reactor> reactor = Reactor::global_remote();
};

Runner::Runner() { ctx_.reset(new RunnerCtx); }

void Runner::start_test(Var<Runnable> test, Callback<Var<Runnable>> fn) {
    locked(ctx_->mutex, [
        ctx = ctx_, tp = std::move(test), fn = std::move(fn)
    ]() {
        // Note: here we MUST force the runnable's reactor to be our reactor
        // otherwise we cannot run the specified test...
        assert(!tp->reactor);
        tp->reactor = ctx->reactor;
        assert(ctx->active >= 0);
        ctx->active += 1;
        debug("runner: scheduling %p", (void *)tp.get());
        ctx->reactor->call_soon([
            ctx, tp = std::move(tp), fn = std::move(fn)
        ]() {
            debug("runner: starting %p", (void *)tp.get());
            tp->begin([ ctx, tp = std::move(tp), fn = std::move(fn) ](Error) {
                // TODO: do not ignore the error
                debug("runner: ending %p", (void *)tp.get());
                tp->end([ ctx, tp = std::move(tp), fn = std::move(fn) ](Error) {
                    // TODO: do not ignore the error
                    debug("runner: cleaning-up %p", (void *)tp.get());
                    // For robustness, delay the final callback to the beginning
                    // of next I/O cycle to prevent possible user after frees.
                    // This could happen because, in our current position on the
                    // stack, we have been called by `Runnable` code that may
                    // use `this` after calling the callback. But this would be
                    // a problem because `test` is most likely to be destroyed
                    // after `fn()` returns. This, when unwinding the stack, the
                    // use after free would happen.
                    ctx->reactor->call_soon(
                        [ ctx, tp = std::move(tp), fn = std::move(fn) ]() {
                            debug("runner: callbacking %p", (void *)tp.get());
                            ctx->active -= 1;
                            debug("runner: #active tasks: %d",
                                  (int)ctx->active);
                            if (ctx->active <= 0) {
                                ctx->reactor->break_loop();
                            }
                            fn(std::move(tp));
                        });
                });
            });
        });
    });
}

void Runner::break_loop_() {
    locked(ctx_->mutex, [&]() { ctx_->reactor->break_loop(); });
}

bool Runner::empty() {
    return locked(ctx_->mutex, [&]() { return ctx_->active == 0; });
}

Runner::~Runner() { break_loop_(); }

/*static*/ Var<Runner> Runner::global() {
    // XXX: how do we protect this singleton?
    static Var<Runner> singleton(new Runner);
    return singleton;
}

} // namespace nettests
} // namespace mk
