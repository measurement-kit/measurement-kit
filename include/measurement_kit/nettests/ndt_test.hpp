// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_NETTESTS_NDT_TEST_HPP
#define MEASUREMENT_KIT_NETTESTS_NDT_TEST_HPP

#include <measurement_kit/nettests/ooni_test.hpp>

namespace mk {
namespace nettests {

class NdtTest : public OoniTest {
  public:
    NdtTest();
    NdtTest(Settings);
  protected:
    void main(std::string, Settings, Callback<report::Entry>) override;
    Var<NetTest> create_test_() override;
};

} // namespace nettests
} // namespace mk
#endif
