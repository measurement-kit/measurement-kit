// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/ndt.hpp>

namespace mk {
namespace ndt {

void NdtTest::begin(Callback<> cb) {
    ndt::run([=](Error) { cb(); }, options, logger, reactor);
}

void NdtTest::end(Callback<> cb) { cb(); }

Var<NetTest> NdtTest::create_test_() {
    NdtTest *test = new NdtTest;
    test->logger = logger;
    test->reactor = reactor;
    test->options = options;
    test->input_filepath = input_filepath;
    test->output_filepath = output_filepath;
    return Var<NetTest>(test);
}

} // namespace mk
} // namespace ndt
