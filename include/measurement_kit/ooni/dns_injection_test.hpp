// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_OONI_DNS_INJECTION_TEST_HPP
#define MEASUREMENT_KIT_OONI_DNS_INJECTION_TEST_HPP

#include <map>
#include <measurement_kit/common/var.hpp>
#include <measurement_kit/common/settings.hpp>
#include <measurement_kit/ooni/base_test.hpp>
#include <string>

namespace mk {

class NetTest;

namespace ooni {

/// Parameters of dns-injection test
class DnsInjectionTest : public BaseTest {
  public:
    /// Default constructor
    DnsInjectionTest() {}

    /// Set backend used to perform the test
    DnsInjectionTest &set_backend(std::string backend) {
        settings["nameserver"] = backend;
        return *this;
    }

    /// Create instance of the test
    Var<NetTest> create_test_() override;
};

} // namespace ooni
} // namespace mk
#endif
