// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "private/nettests/runnable.hpp"

#include <measurement_kit/nettests.hpp>
#include <measurement_kit/ooni.hpp>

namespace mk {
namespace nettests {

DnsInjectionTest::DnsInjectionTest() : BaseTest() {
    runnable.reset(new DnsInjectionRunnable);
    runnable->test_name = "dns_injection";
    runnable->test_version = "0.0.1";
    runnable->needs_input = true;
}

void DnsInjectionRunnable::main(std::string input, Settings options,
                                Callback<SharedPtr<report::Entry>> cb) {
    ooni::dns_injection(input, options, cb, reactor, logger);
}

} // namespace nettests
} // namespace mk
