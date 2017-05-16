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
    runnable->test_helpers_names = {"backend"};
}

void WebConnectivityRunnable::main(std::string input, Settings options,
                                   Callback<Var<report::Entry>> cb) {
    ooni::web_connectivity(input, options, cb, reactor, logger);
}

void WebConnectivityRunnable::fixup_entry(report::Entry &entry) {
    try {
        auto backend = entry["test_helpers"]["backend"].get<std::string>();
        if (mk::startswith(backend, "https://")) {
            entry["test_helpers"]["backend"] = {{
                "address", backend
            }, {
                "type", "https"
            }};
        } else {
            /* TODO: Here we should deal with this case. Or, even better,
                     we should probably enhance the model such that the backend
                     isn't a string but a more structured object. This will
                     probably happen when we finish the cloudfronted code. */
            logger->warn("We are sending a string-only backend entry.");
        }
    } catch (const std::exception &exc) {
        logger->warn("Cannot fixup entry: %s", exc.what());
    }
}

} // namespace nettests
} // namespace mk
