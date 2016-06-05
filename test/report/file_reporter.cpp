// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <ctime>
#include <measurement_kit/ext.hpp>
#include <measurement_kit/report.hpp>
#include "src/common/utils.hpp"

using namespace mk::report;
using namespace mk;
using json = nlohmann::json;

TEST_CASE("The constructor works correctly") {
    REQUIRE_NOTHROW(FileReporter());
}

TEST_CASE("open() tells us if it encounters an error") {
    FileReporter reporter;
    // This should cause failure on open() because directory doesn't exist
    reporter.filename = "/nonexistent/foobar.json";
    REQUIRE(reporter.open() != NoError());
}

// TODO: how to test failure of write and close?

TEST_CASE(
    "It should be possible to write multiple entries to an open report") {
        const std::string input = "some input";

        mk::Settings options;
        options["opt1"] = "value1";
        options["opt2"] = "value2";

        FileReporter reporter;
        reporter.test_name = "example_test";
        reporter.test_version = MEASUREMENT_KIT_VERSION;
        reporter.filename = "example_test_report.json";
        reporter.options = options;
        mk::utc_time_now(&reporter.test_start_time);

        mk::report::Entry entry;
        entry["input"] = input;
        entry["antani"] = "fuffa";
        REQUIRE(reporter.open() == NoError());
        REQUIRE(reporter.write_entry(entry) == NoError());
        REQUIRE(reporter.close() == NoError());

        std::ifstream infile(reporter.filename);
        for (std::string line; getline(infile, line);) {
            json entry = json::parse(line.c_str());
            REQUIRE(entry["test_name"].get<std::string>() ==
                    reporter.test_name);
            REQUIRE(entry["test_version"].get<std::string>() ==
                    reporter.test_version);
            REQUIRE(entry["probe_ip"].get<std::string>() == reporter.probe_ip);

            REQUIRE(entry["software_name"].get<std::string>() ==
                    "measurement_kit");
            REQUIRE(entry["software_version"].get<std::string>() ==
                    MEASUREMENT_KIT_VERSION);
            REQUIRE(entry["data_format_version"].get<std::string>() == "0.2.0");

            // Check that the first report entry is correct.
            REQUIRE(entry["input"].get<std::string>() == input);
            REQUIRE(entry["antani"].get<std::string>() == "fuffa");
        }
    }
