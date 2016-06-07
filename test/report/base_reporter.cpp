// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN

#include "src/ext/Catch/single_include/catch.hpp"
#include <measurement_kit/report.hpp>

using namespace mk;
using namespace mk::report;

TEST_CASE("The constructor works correctly") {
    REQUIRE_NOTHROW(BaseReporter());
}

TEST_CASE("The open() method works correctly") {
    BaseReporter reporter;
    REQUIRE(reporter.open() == NoError());
    REQUIRE(reporter.open() == ReportAlreadyOpen());
}

TEST_CASE("The write_entry() method works correctly") {
    BaseReporter reporter;
    Entry entry;
    REQUIRE(reporter.write_entry(entry) == ReportNotOpen());
    REQUIRE(reporter.open() == NoError());
    REQUIRE(reporter.write_entry(entry) == NoError());
    REQUIRE(reporter.write_entry(entry) == NoError());
    REQUIRE(reporter.write_entry(entry) == NoError());
    REQUIRE(reporter.close() == NoError());
    REQUIRE(reporter.write_entry(entry) == ReportAlreadyClosed());
}

TEST_CASE("The close() method works correctly") {
    BaseReporter reporter;
    REQUIRE(reporter.open() == NoError());
    REQUIRE(reporter.close() == NoError());
    REQUIRE(reporter.close() == ReportAlreadyClosed());
}
