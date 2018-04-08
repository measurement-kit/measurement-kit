// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/libmeasurement_kit/ext/catch.hpp"

#include "src/libmeasurement_kit/common/utils.hpp"

#include "src/libmeasurement_kit/report/file_reporter.hpp"

using namespace mk::report;
using namespace mk;

TEST_CASE("The constructor works correctly") {
    REQUIRE_NOTHROW(FileReporter::make("/nonexistent/foobar.njson"));
}

TEST_CASE("open() tells us if it encounters an error") {
    Report report;
    SharedPtr<BaseReporter> reporter = FileReporter::make("/nonexistent/foobar.njson");
    // This should cause failure on open() because directory doesn't exist
    reporter->open(report)([](Error err) {
        REQUIRE(err);
    });
}

// TODO: how to test failure of write and close?

TEST_CASE(
    "It should be possible to write multiple entries to an open report") {
        const std::string input = "some input";

        mk::Settings options;
        options["opt1"] = "value1";
        options["opt2"] = "value2";

        Report report;
        report.test_name = "example_test";
        report.test_version = MEASUREMENT_KIT_VERSION;
        report.options = options;
        mk::utc_time_now(&report.test_start_time);
        std::string filename("example_test_report.njson");
        report.add_reporter(FileReporter::make(filename));

        mk::report::Entry entry;
        entry["input"] = input;
        entry["antani"] = "fuffa";
        report.fill_entry(entry);
        report.open([&](Error err) {
            REQUIRE(!err);
            report.write_entry(entry, [&](Error err) {
                REQUIRE(!err);
                report.close([&](Error err) {
                    REQUIRE(!err);

                    std::ifstream infile(filename);
                    for (std::string line; getline(infile, line);) {
                        Json entry = Json::parse(line.c_str());
                        REQUIRE(entry["test_name"].get<std::string>() ==
                                report.test_name);
                        REQUIRE(entry["test_version"].get<std::string>() ==
                                report.test_version);
                        REQUIRE(entry["probe_ip"].get<std::string>() ==
                                report.probe_ip);

                        REQUIRE(entry["software_name"].get<std::string>() ==
                                "measurement_kit");
                        REQUIRE(entry["software_version"].get<std::string>() ==
                                MEASUREMENT_KIT_VERSION);
                        REQUIRE(entry["data_format_version"].get<std::string>()
                                == "0.2.0");

                        // Check that the first report entry is correct.
                        REQUIRE(entry["input"].get<std::string>() == input);
                        REQUIRE(entry["antani"].get<std::string>() == "fuffa");
                    }
                });
            }, Logger::global());
        });
    }
