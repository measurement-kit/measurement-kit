// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_OONI_HTTP_INVALID_REQUEST_LINE_HPP
#define MEASUREMENT_KIT_OONI_HTTP_INVALID_REQUEST_LINE_HPP

#include <measurement_kit/common.hpp>
#include <measurement_kit/ooni/ooni_test.hpp>
#include <measurement_kit/report.hpp>

namespace mk {
namespace ooni {

using namespace mk::report;

void http_invalid_request_line(Settings, Callback<Var<report::Entry>>,
                               Var<Reactor> = Reactor::global(),
                               Var<Logger> = Logger::global());

class HttpInvalidRequestLine : public OoniTest {
  public:
    HttpInvalidRequestLine() : HttpInvalidRequestLine({}) {}
    HttpInvalidRequestLine(Settings o) : OoniTest("", o) {
        test_name = "http_invalid_request_line";
        test_version = "0.0.1";
    }

    void main(std::string, Settings options,
              Callback<report::Entry> cb) override {
        http_invalid_request_line(options, [=](Var<report::Entry> e) {
             cb(*e);
        }, reactor, logger);
    }

    Var<NetTest> create_test_() override;
};

} // namespace ooni
} // namespace mk
#endif
