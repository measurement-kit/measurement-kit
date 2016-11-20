// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/nettests.hpp>
#include <measurement_kit/ooni.hpp>

namespace mk {
namespace nettests {

WebConnectivityTest::WebConnectivityTest() : BaseTest() {
    runnable.reset(new WebConnectivityRunnable);
    runnable->test_name = "web_connectivity";
    runnable->test_version = "0.0.1";
    runnable->needs_input = true;
}

void WebConnectivityRunnable::main(std::string input, Settings options,
                                   Callback<report::Entry> cb) {
    ooni::web_connectivity(input, options, [=](Var<report::Entry> e) {
         cb(*e);
    }, reactor, logger);
}

} // namespace nettests
} // namespace mk
