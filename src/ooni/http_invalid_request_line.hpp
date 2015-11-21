// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_OONI_HTTP_INVALID_REQUEST_LINE_HPP
#define MEASUREMENT_KIT_OONI_HTTP_INVALID_REQUEST_LINE_HPP

#include <measurement_kit/common/utils.hpp>
#include "src/ooni/errors.hpp"
#include "src/ooni/net_test.hpp"
#include "src/ooni/http_test.hpp"
#include <sys/stat.h>

namespace measurement_kit {
namespace ooni {

using namespace measurement_kit::common;
using namespace measurement_kit::report;

class HTTPInvalidRequestLine : public HTTPTest {
    using HTTPTest::HTTPTest;

    int tests_run = 0;

    std::function<void(report::Entry)> callback;

  public:
    HTTPInvalidRequestLine(Settings options_) : HTTPTest(options_) {
        test_name = "http_invalid_request_line";
        test_version = "0.0.1";
    };

    void main(Settings options, std::function<void(report::Entry)> &&cb);
};
}
}
#endif
