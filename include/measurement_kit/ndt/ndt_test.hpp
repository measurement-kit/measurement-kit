// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_NDT_NDT_TEST_HPP
#define MEASUREMENT_KIT_NDT_NDT_TEST_HPP

#include <measurement_kit/common.hpp>

namespace mk {
namespace ndt {

class NdtTest : public NetTest {
  public:
    using NetTest::NetTest;
    void begin(Callback<>) override;
    void end(Callback<>) override;
    Var<NetTest> create_test_() override;
};

} // namespace ndt
} // namespace mk
#endif
