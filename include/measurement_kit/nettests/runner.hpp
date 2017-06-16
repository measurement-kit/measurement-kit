// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_NETTESTS_RUNNER_HPP
#define MEASUREMENT_KIT_NETTESTS_RUNNER_HPP

#include <measurement_kit/common.hpp>

namespace mk {
namespace nettests {
class Runnable;

class Runner : public HasGlobalFactory<Runner> {
  public:
    void start_test(Var<Runnable> test, Callback<Var<Runnable>> func);
    void stop();
    bool empty();

  private:
    AsyncRunner impl_;
};

} // namespace nettests
} // namespace mk
#endif
