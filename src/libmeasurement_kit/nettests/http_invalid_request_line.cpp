// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/nettests.hpp>
#include <measurement_kit/ooni.hpp>

namespace mk {
namespace nettests {

HttpInvalidRequestLine::HttpInvalidRequestLine() : HttpInvalidRequestLine({}) {}

HttpInvalidRequestLine::HttpInvalidRequestLine(Settings o) : OoniTest("", o) {
    test_name = "http_invalid_request_line";
    test_version = "0.0.1";
}

void HttpInvalidRequestLine::main(std::string, Settings options,
                                  Callback<report::Entry> cb) {
    ooni::http_invalid_request_line(options, [=](Var<report::Entry> e) {
         cb(*e);
    }, reactor, logger);
}

Var<NetTest> HttpInvalidRequestLine::create_test_() {
    HttpInvalidRequestLine *test = new HttpInvalidRequestLine(options);
    test->logger = logger;
    test->reactor = reactor;
    test->output_filepath = output_filepath;
    test->entry_cb = entry_cb;
    test->begin_cb = begin_cb;
    test->end_cb = end_cb;
    return Var<NetTest>(test);
}

} // namespace nettests
} // namespace mk
