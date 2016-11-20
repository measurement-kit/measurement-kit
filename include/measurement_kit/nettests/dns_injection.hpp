// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_NETTESTS_DNS_INJECTION_HPP
#define MEASUREMENT_KIT_NETTESTS_DNS_INJECTION_HPP

#include <measurement_kit/nettests/ooni_test.hpp>

namespace mk {
namespace nettests {

class DnsInjection : public OoniTest {
  public:
    DnsInjection();
    DnsInjection(std::string f, Settings o);
    void main(std::string, Settings, Callback<report::Entry>) override;
    Var<NetTest> create_test_() override;
};

} // namespace nettests
} // namespace mk
#endif
