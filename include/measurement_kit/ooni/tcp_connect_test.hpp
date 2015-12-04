// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_OONI_TCP_CONNECT_TEST_HPP
#define MEASUREMENT_KIT_OONI_TCP_CONNECT_TEST_HPP

#include <measurement_kit/common/net_test.hpp>
#include <measurement_kit/common/settings.hpp>
#include <measurement_kit/common/var.hpp>
#include <measurement_kit/ooni/base_test.hpp>
#include <string>

namespace mk {
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

    /// Set input file path
    TcpConnectTest &set_input_file_path(std::string ifp) {
        input_path = ifp;
        return *this;
    }

    /// Set verbose
    TcpConnectTest &set_verbose() {
        is_verbose = true;
        return *this;
    }

    /// Set log-message handler
    TcpConnectTest &on_log(std::function<void(const char *)> func) {
        log_handler = func;
        return *this;
    }

    /// Create instance of the test
    Var<NetTest> create_test_() override;

    Settings settings;
    bool is_verbose = false;
    std::function<void(const char *)> log_handler;
    std::string input_path;
};

} // namespace ooni
} // namespace mk
#endif
