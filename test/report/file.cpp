#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <ctime>
#include "report/file.hpp"
#include "report/entry.hpp"

TEST_CASE("The constructor for [FileReport] works correctly", "[BaseReport]") {
  time_t start_time = time(0);

  std::map<std::string, std::string> options;
  options["opt1"] = "value1";
  options["opt2"] = "value2";

  REQUIRE_NOTHROW(FileReporter reporter = FileReporter(
      "example_test", "0.0.1", start_time, "127.0.0.1",
      options, "example_test_report.yaml"
  ));
}

TEST_CASE("Report lifecycle", "[BaseReport]") {
  SECTION("It should be possible to write multiple entries to an open report") {
    const std::string report_file = "example_test_report.yaml";
    const std::string test_name = "example_test";
    const std::string test_version = "0.0.1";
    const std::string probe_ip = "127.0.0.1";

    const std::string input = "some input";

    time_t start_time = time(0);
    std::map<std::string, std::string> options;
    options["opt1"] = "value1";
    options["opt2"] = "value2";

    FileReporter reporter = FileReporter(
      test_name, test_version, start_time, probe_ip,
      options, report_file
    );
    auto entry = ReportEntry(input);
    entry["antani"] = "fuffa";
    reporter.open();
    reporter.writeEntry(entry);
    reporter.close();
    
    std::vector<YAML::Node> entries = YAML::LoadAllFromFile(report_file);

    // Check that the report header is correct
    REQUIRE(entries[0]["test_name"].as<std::string>() == test_name);
    REQUIRE(entries[0]["test_version"].as<std::string>() == test_version);
    REQUIRE(entries[0]["probe_ip"].as<std::string>() == probe_ip);

    REQUIRE(entries[0]["software_name"].as<std::string>() == "ight");
    REQUIRE(entries[0]["software_version"].as<std::string>() == "0.0.1");
    REQUIRE(entries[0]["data_format_version"].as<std::string>() == "0.1");
      
    // Check that the first report entry is correct.
    REQUIRE(entries[1]["input"].as<std::string>() == input);
    REQUIRE(entries[1]["antani"].as<std::string>() == "fuffa");

  }
}
