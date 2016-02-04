// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_OONI_TCP_CONNECT_TEST_HPP
#define MEASUREMENT_KIT_OONI_TCP_CONNECT_TEST_HPP

#include <map>
#include <measurement_kit/common/settings.hpp>
#include <measurement_kit/common/var.hpp>
#include <measurement_kit/ooni/base_test.hpp>
#include <string>

namespace mk {

class NetTest;

namespace ooni {

/// Parameters of tcp-connect test
class TcpConnectTest : public BaseTest {
  public:
    /// Default constructor
    TcpConnectTest() {}

    /// Set backend used to perform the test
    TcpConnectTest &set_port(std::string port) {
        settings["port"] = port;
        return *this;
    }

    /// Create instance of the test
    Var<NetTest> create_test_() override;
};

} // namespace ooni
} // namespace mk
#endif
