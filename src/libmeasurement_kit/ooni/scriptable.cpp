// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/ooni.hpp>

namespace mk {
namespace ooni {
namespace scriptable {

// Convenience macro used to implement all callbacks below
#define XX                                                                     \
    [=](Var<Entry> entry) {                                                    \
        complete([=]() {                                                       \
            if (!entry) {                                                      \
                callback("{}");                                                \
                return;                                                        \
            }                                                                  \
            callback(entry->dump(4));                                          \
        });                                                                    \
    }

void dns_injection(std::string input, Settings settings,
                   Callback<std::string> callback, Var<Runner> runner,
                   Var<Logger> logger) {
    runner->run([=](Continuation<> complete) {
        ooni::dns_injection(input, settings, XX, runner->reactor, logger);
    });
}

void http_invalid_request_line(
        Settings settings, Callback<std::string> callback,
        Var<Runner> runner, Var<Logger> logger) {
    runner->run([=](Continuation<> complete) {
        ooni::http_invalid_request_line(settings, XX, runner->reactor, logger);
    });
}

void tcp_connect(std::string input, Settings settings,
                 Callback<std::string> callback,
                 Var<Runner> runner, Var<Logger> logger) {
    runner->run([=](Continuation<> complete) {
        ooni::tcp_connect(input, settings, XX, runner->reactor, logger);
    });
}

void web_connectivity(std::string input, Settings settings,
                      Callback<std::string> callback,
                      Var<Runner> runner,
                      Var<Logger> logger) {
    runner->run([=](Continuation<> complete) {
        ooni::web_connectivity(input, settings, XX, runner->reactor, logger);
    });
}

} // namespace scriptable
} // namespace mk
} // namespace ooni
