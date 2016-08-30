// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/ndt.hpp>
#include <measurement_kit/ext.hpp>
#include "../common/utils.hpp"

namespace mk {
namespace ndt {

using json = nlohmann::json;
using namespace mk::report;

NdtTest::NdtTest(Settings s) : OoniTest("", s) {
    options["save_real_probe_ip"] = true;
    test_name = "ndt";
    test_version = "0.0.2";
}

static void log_summary(Var<Logger> logger, Var<Entry> entry) {
    try {
        nlohmann::json root{
            {"type", "summary"},
        };
        root["failure"] = (*entry)["failure"];
        for (auto measurement: (*entry)["test_s2c"]) {
            nlohmann::json child;
            child["num_streams"] = measurement["params"]["num_streams"];
            child["connect_times"] = measurement["connect_times"];
            child["min_rtt"] = {
                measurement["web100_data"]["MinRTT"],
                "ms"
            };
            std::vector<double> speeds;
            for (auto e: measurement["receiver_data"]) {
                speeds.push_back(e[1]);
            }
            child["median_speed"] = {
                percentile(speeds, 0.5),
                "kbit/s"
            };
            child["10_percentile_speed"] = {
                percentile(speeds, 0.1),
                "kbit/s"
            };
            child["90_percentile_speed"] = {
                percentile(speeds, 0.9),
                "kbit/s"
            };
            root["test_s2c"].push_back(child);
        }
        logger->log(MK_LOG_INFO | MK_LOG_JSON, "%s", root.dump().c_str());
    } catch (const std::exception &e) {
        logger->warn("could not write NDT test summary: %s", e.what());
        /* suppress */ ;
    }
}

void NdtTest::main(std::string, Settings settings, Callback<Entry> cb) {
    Var<Entry> entry(new Entry);
    (*entry)["failure"] = nullptr;
    // Note: `options` is the class attribute and `settings` is instead a
    // possibly modified copy of the `options` object
    ndt::run(entry, [=](Error error) {
        if (error) {
            (*entry)["failure"] = error.as_ooni_error();
        }
        log_summary(logger, entry);
        // XXX The callback should probably take a Var<Entry>
        cb(*entry);
    }, settings, logger, reactor);
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

} // namespace mk
} // namespace ndt
