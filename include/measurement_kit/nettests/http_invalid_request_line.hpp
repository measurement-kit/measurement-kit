// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_NETTESTS_HTTP_INVALID_REQUEST_LINE_HPP
#define MEASUREMENT_KIT_NETTESTS_HTTP_INVALID_REQUEST_LINE_HPP

#include <measurement_kit/nettests/ooni_test.hpp>
#include <measurement_kit/ooni.hpp>

namespace mk {
namespace nettests {

class HttpInvalidRequestLine : public OoniTest {
  public:
    HttpInvalidRequestLine() : HttpInvalidRequestLine({}) {}
    HttpInvalidRequestLine(Settings o) : OoniTest("", o) {
        test_name = "http_invalid_request_line";
        test_version = "0.0.1";
    }

    void main(std::string, Settings options, Callback<report::Entry> cb) override {
        ooni::http_invalid_request_line(options, [=](Var<report::Entry> e) {
             cb(*e);
        }, reactor, logger);
    }

    Var<NetTest> create_test_() override;
};

} // namespace nettests
} // namespace mk
#endif
