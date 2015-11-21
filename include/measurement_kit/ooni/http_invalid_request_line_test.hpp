// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_OONI_HTTP_INVALID_REQUEST_LINE_TEST_HPP
#define MEASUREMENT_KIT_OONI_HTTP_INVALID_REQUEST_LINE_TEST_HPP

#include <measurement_kit/common/net_test.hpp>
#include <measurement_kit/common/settings.hpp>
#include <measurement_kit/common/var.hpp>
#include <string>

namespace measurement_kit {
namespace ooni {

/// Parameters of http-invalid-request-line test
class HttpInvalidRequestLineTest {
  public:
    /// Constructor with settings
    HttpInvalidRequestLineTest(common::Settings s) : settings(s) {}

    /// Create instance of the test
    common::Var<common::NetTest> create_test();

    common::Settings settings;       ///< Test settings
};

} // namespace ooni
} // namespace measurement_kit
#endif
