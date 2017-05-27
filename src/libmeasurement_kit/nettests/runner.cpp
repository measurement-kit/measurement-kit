// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/nettests.hpp>

namespace mk {
namespace nettests {

struct RunnerCtx {
    Var<Reactor> reactor = Reactor::global_remote();
};

Runner::Runner() { ctx_.reset(new RunnerCtx); }

void Runner::start_test(Var<Runnable> test, Callback<Var<Runnable>> fn) {
    ctx_->reactor->run_task_deferred(
        "runner",
        test->logger,
        [=](Callback<> &&cb) {
            test->begin([=](Error /*err*/) {
                // TODO: do we want to ignore the error?
                test->end([=](Error /*err*/) {
                    // TODO: do we want to ignore the error?
                    cb();
                });
            });
        },
        [=](auto /*callback*/) {
            fn(test);
        });
}

void Runner::break_loop_() { ctx_->reactor->break_loop(); }

bool Runner::empty() { return !ctx_->reactor->is_running(); }

void Runner::join_() {
    // NOTHING
}

Runner::~Runner() {
    break_loop_();
    join_();
}

/*static*/ Var<Runner> Runner::global() {
    return locked_global([]() {
        static Var<Runner> singleton(new Runner);
        return singleton;
    });
}

} // namespace nettests
} // namespace mk
