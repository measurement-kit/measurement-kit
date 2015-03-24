#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <ctime>
#include <ight/report/file.hpp>
#include <ight/report/entry.hpp>

using namespace ight::report::file;

TEST_CASE("The constructor for [FileReport] works correctly", "[BaseReport]") {
  REQUIRE_NOTHROW(FileReporter());
}

TEST_CASE("Report lifecycle", "[BaseReport]") {
  SECTION("It should be possible to write multiple entries to an open report") {
    const std::string input = "some input";

    std::map<std::string, std::string> options;
    options["opt1"] = "value1";
    options["opt2"] = "value2";

    FileReporter reporter;
    reporter.test_name = "example_test";
    reporter.test_version = "0.0.1";
    reporter.filename = "example_test_report.yaml";
    reporter.options = options;
    reporter.start_time = time(0);

    auto entry = ReportEntry(input);
    entry["antani"] = "fuffa";
    reporter.open();
    reporter.writeEntry(entry);
    reporter.close();
    
    std::vector<YAML::Node> entries = YAML::LoadAllFromFile(reporter.filename);

    // Check that the report header is correct
    REQUIRE(entries[0]["test_name"].as<std::string>() == reporter.test_name);
    REQUIRE(entries[0]["test_version"].as<std::string>() == reporter.test_version);
    REQUIRE(entries[0]["probe_ip"].as<std::string>() == reporter.probe_ip);

    REQUIRE(entries[0]["software_name"].as<std::string>() == "ight");
    REQUIRE(entries[0]["software_version"].as<std::string>() == "0.0.1");
    REQUIRE(entries[0]["data_format_version"].as<std::string>() == "0.1");
      
    // Check that the first report entry is correct.
    REQUIRE(entries[1]["input"].as<std::string>() == input);
    REQUIRE(entries[1]["antani"].as<std::string>() == "fuffa");

  }
}
