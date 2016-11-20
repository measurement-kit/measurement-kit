// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/nettests.hpp>

namespace mk {
namespace nettests {

Var<NetTest> DnsInjection::create_test_() {
    DnsInjection *test = new DnsInjection(input_filepath, options);
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
