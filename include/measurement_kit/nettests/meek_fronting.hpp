// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_NETTESTS_MEEK_FRONTING_HPP
#define MEASUREMENT_KIT_NETTESTS_MEEK_FRONTING_HPP

#include <measurement_kit/nettests/base_test.hpp>

namespace mk {
namespace nettests {

class MeekFrontingTest : public BaseTest {
  public:
    MeekFrontingTest();
};

class MeekFrontingRunnable : public Runnable {
  public:
    void main(std::string, Settings, Callback<Var<report::Entry>>) override;
};

} // namespace nettests
} // namespace mk
#endif
