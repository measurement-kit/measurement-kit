// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <ctime>
#include <measurement_kit/ext.hpp>
#include "src/report/file_reporter.hpp"
#include "src/common/utils.hpp"

using namespace mk::report;
using json = nlohmann::json;

TEST_CASE("The constructor for [FileReport] works correctly", "[BaseReport]") {
    REQUIRE_NOTHROW(FileReporter());
}

TEST_CASE("Report lifecycle", "[BaseReport]") {
    SECTION(
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

        json entry;
        entry["input"] = input;
        entry["antani"] = "fuffa";
        reporter.open();
        reporter.write_entry(entry);
        reporter.close();

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
}
