// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_OONI_TCP_CONNECT_TEST_HPP
#define MEASUREMENT_KIT_OONI_TCP_CONNECT_TEST_HPP

#include <measurement_kit/common/net_test.hpp>
#include <measurement_kit/common/settings.hpp>
#include <measurement_kit/common/var.hpp>
#include <string>

namespace measurement_kit {
namespace ooni {

/// Parameters of tcp-connect test
class TcpConnectTest {
  public:
    /// Constructor with settings and input-path
    TcpConnectTest(common::Settings s, std::string f)
            : settings(s), input_file_path(f) {}

    /// Create instance of the test
    common::Var<common::NetTest> create_test();

    common::Settings settings;       ///< Test settings
    std::string input_file_path;     ///< Input-file path
};

} // namespace ooni
} // namespace measurement_kit
#endif
