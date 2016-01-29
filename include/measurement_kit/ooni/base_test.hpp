// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_OONI_BASE_TEST_HPP
#define MEASUREMENT_KIT_OONI_BASE_TEST_HPP

#include <measurement_kit/common/var.hpp>
#include <measurement_kit/common/settings.hpp>
#include <functional>
#include <string>

namespace mk {

class NetTest;

namespace ooni {

/// Base class for network tests
class BaseTest {
  public:
    BaseTest() {}          ///< Default constructor
    virtual ~BaseTest() {} ///< Default destructor

    /// Set input file path
    BaseTest &set_input_file_path(std::string ifp) {
        input_path = ifp;
        return *this;
    }

    /// Set output file path
    BaseTest &set_output_file_path(std::string ofp) {
        output_path = ofp;
        return *this;
    }

    /// Set verbose
    BaseTest &set_verbose(bool verbose = true) {
        is_verbose = verbose;
        return *this;
    }

    /// Set log-message handler
    BaseTest &on_log(std::function<void(const char *)> func) {
        log_handler = func;
        return *this;
    }

    /// Create instance of the test
    virtual Var<NetTest> create_test_() {
        return Var<NetTest>();
    }

    /// Run synchronous test
    void run();

    /// Run asynchronous test
    void run(std::function<void()> callback);

    Settings settings;                             ///< Other test settings
    bool is_verbose = false;                       ///< Be verbose
    std::function<void(const char *)> log_handler; ///< Log handler func
    std::string input_path;                        ///< Input file path
    std::string output_path;                       ///< Output file path
};

} // namespace ooni
} // namespace mk
#endif
