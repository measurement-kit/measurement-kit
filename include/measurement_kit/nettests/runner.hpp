// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_NETTESTS_RUNNER_HPP
#define MEASUREMENT_KIT_NETTESTS_RUNNER_HPP

#include <measurement_kit/nettests/runnable.hpp>

namespace mk {
namespace nettests {
struct RunnerCtx;

class Runner {
  public:
    Runner();
    void start_test(Var<Runnable> test, Callback<Var<Runnable>> func);
    void break_loop_();
    bool empty();
    void join_();
    ~Runner();
    static Var<Runner> global();

  private:
    Var<RunnerCtx> ctx_;
};

} // namespace nettests
} // namespace mk
#endif
