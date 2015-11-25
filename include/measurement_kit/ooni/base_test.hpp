// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_OONI_BASE_TEST_HPP
#define MEASUREMENT_KIT_OONI_BASE_TEST_HPP

#include <measurement_kit/common/var.hpp>
#include <functional>

namespace measurement_kit {

namespace common {
class NetTest;
}

namespace ooni {

/// Base class for network tests
class BaseTest {
  public:
    BaseTest() {}          ///< Default constructor
    virtual ~BaseTest() {} ///< Default destructor

    /// Create instance of the test
    virtual common::Var<common::NetTest> create_test_() {
        return common::Var<common::NetTest>();
    }

    /// Run synchronous test
    void run();

    /// Run asynchronous test
    void run(std::function<void()> callback);
};

} // namespace ooni
} // namespace measurement_kit
#endif
