// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_OONI_DNS_INJECTION_TEST_HPP
#define MEASUREMENT_KIT_OONI_DNS_INJECTION_TEST_HPP

#include <measurement_kit/common/net_test.hpp>
#include <measurement_kit/common/settings.hpp>
#include <measurement_kit/common/var.hpp>
#include <string>

namespace measurement_kit {
namespace ooni {

/// Parameters of dns-injection test
class DnsInjectionTest {
  public:

    /// Default constructor
    DnsInjectionTest() {}

    /// Set backend used to perform the test
    DnsInjectionTest &set_nameserver(std::string nameserver) {
        settings["nameserver"] = nameserver;
        return *this;
    }

    /// Set input file path
    DnsInjectionTest &set_input_file_path(std::string ifp) {
        input_path = ifp;
        return *this;
    }

    /// Set verbose
    DnsInjectionTest &set_verbose() {
        is_verbose = true;
        return *this;
    }

    /// Set log-message handler
    DnsInjectionTest &on_log(std::function<void(const char *)> func) {
        log_handler = func;
        return *this;
    }

    /// Create instance of the test
    common::Var<common::NetTest> create_test();

    common::Settings settings;
    bool is_verbose = false;
    std::function<void(const char *)> log_handler;
    std::string input_path;
};

} // namespace ooni
} // namespace measurement_kit
#endif
