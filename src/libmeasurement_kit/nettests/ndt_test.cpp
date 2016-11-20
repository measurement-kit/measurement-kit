// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/nettests.hpp>
#include <measurement_kit/ndt.hpp>

namespace mk {
namespace nettests {

NdtTest::NdtTest(Settings s) : OoniTest("", s) {
    options["save_real_probe_ip"] = true;
    test_name = "ndt";
    test_version = "0.0.4";
}

void NdtTest::main(std::string, Settings settings, Callback<report::Entry> cb) {
    Var<report::Entry> entry(new report::Entry);
    (*entry)["failure"] = nullptr;
    // Note: `options` is the class attribute and `settings` is instead a
    // possibly modified copy of the `options` object
    ndt::run(entry, [=](Error error) {
        if (error) {
            (*entry)["failure"] = error.as_ooni_error();
        }
        // XXX The callback should probably take a Var<report::Entry>
        cb(*entry);
    }, settings, reactor, logger);
}

Var<NetTest> NdtTest::create_test_() {
    NdtTest *test = new NdtTest;
    test->logger = logger;
    test->reactor = reactor;
    test->options = options;
    test->input_filepath = input_filepath;
    test->output_filepath = output_filepath;
    test->entry_cb = entry_cb;
    test->begin_cb = begin_cb;
    test->end_cb = end_cb;
    return Var<NetTest>(test);
}

} // namespace nettests
} // namespace mk
