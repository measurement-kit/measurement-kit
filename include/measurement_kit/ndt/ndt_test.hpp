// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_NDT_NDT_TEST_HPP
#define MEASUREMENT_KIT_NDT_NDT_TEST_HPP

#include <measurement_kit/ooni.hpp>

namespace mk {
namespace ndt {

using namespace mk::ooni;
using namespace mk::report;

class NdtTest : public OoniTest {
  public:
    NdtTest() : NdtTest(Settings()) {}
    NdtTest(Settings s);
  protected:
    void main(std::string, Settings, Callback<Entry>) override;
    Var<NetTest> create_test_() override;
};

} // namespace ndt
} // namespace mk
#endif
