// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/nettests.hpp>
#include <measurement_kit/ooni.hpp>

namespace mk {
namespace nettests {

WebConnectivity::WebConnectivity() : WebConnectivity("", {}) {}

WebConnectivity::WebConnectivity(std::string f, Settings o) : OoniTest(f, o) {
    test_name = "web_connectivity";
    test_version = "0.0.1";
    needs_input = true;
}

void WebConnectivity::main(std::string input, Settings options,
                           Callback<report::Entry> cb) {
    ooni::web_connectivity(input, options, [=](Var<report::Entry> e) {
         cb(*e);
    }, reactor, logger);
}

Var<NetTest> WebConnectivity::create_test_() {
  WebConnectivity *test = new WebConnectivity(input_filepath, options);
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
