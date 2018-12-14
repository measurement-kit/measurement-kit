// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/common/utils.hpp"
#include "src/libmeasurement_kit/nettests/runnable.hpp"
#include "src/libmeasurement_kit/ooni/nettests.hpp"
#include "src/libmeasurement_kit/http/http.hpp"

namespace mk {
namespace nettests {

WebConnectivityRunnable::WebConnectivityRunnable() noexcept {
    test_name = "web_connectivity";
    test_version = "0.0.1";
    needs_input = true;
    test_helpers_data = {{"web-connectivity", "backend"}};
}

void WebConnectivityRunnable::main(std::string input, Settings options,
                                   Callback<SharedPtr<nlohmann::json>> cb) {
    ooni::web_connectivity(input, options, cb, reactor, logger);
}

void WebConnectivityRunnable::fixup_entry(nlohmann::json &entry) {
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

std::deque<std::string>
WebConnectivityRunnable::fixup_inputs(std::deque<std::string> &&il) {
    std::deque<std::string> rv;
    while (!il.empty()) {
        std::string original_url;
        std::swap(original_url, il.front());
        il.pop_front();
        ErrorOr<http::Url> maybe_url = http::parse_url_noexcept(original_url);
        if (maybe_url.as_error() != NoError()) {
            // Incorrect URL. WebConnectivity will complain for us later.
            rv.push_back(std::move(original_url));
            continue;
        }
        if (maybe_url->schema == "https" && maybe_url->port == 443) {
            // "Test both http and https versions of a website" #1560
            http::Url httpURL = *maybe_url;
            httpURL.schema = "http";
            httpURL.port = 80;
            rv.push_back(httpURL.str());
            // FALLTHROUGH
        }
        // return to the app the original URL rather than a possibly
        // canonicalised version of it; see #1685.
        rv.push_back(original_url);
    }
    return rv;
}

} // namespace nettests
} // namespace mk
