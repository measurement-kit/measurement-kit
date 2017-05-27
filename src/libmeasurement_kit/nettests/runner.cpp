// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/nettests.hpp>

#include <atomic>
#include <cassert>

namespace mk {
namespace nettests {

struct RunnerCtx {
    Var<Reactor> reactor = Reactor::global_remote();
};

Runner::Runner() { ctx_.reset(new RunnerCtx); }

void Runner::start_test(Var<Runnable> test, Callback<Var<Runnable>> fn) {
    reactor->run_task("runner", test->logger,
                      [test = std::move(test)](Callback && cb) {
                          test->begin([test, cb](Error) {
                              // TODO: do not ignore the error
                              test->end([test, cb](Error) {
                                  // TODO: do not ignore the error
                                  cb();
                              });
                          });
                      },
                      fn);
}

void Runner::break_loop_() { ctx_->reactor->break_loop(); }

void Runner::join_() {
    // NOTHING
}

Runner::~Runner() {
    break_loop_();
    join_();
}

bool Runner::empty() { return !ctx_->reactor->is_running(); }

/*static*/ Var<Runner> Runner::global() {
    return locked_global([]() {
        static Var<Runner> singleton(new Runner);
        return singleton;
    });
}

} // namespace nettests
} // namespace mk
