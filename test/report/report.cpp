// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN

#include "src/libmeasurement_kit/ext/Catch/single_include/catch.hpp"
#include <measurement_kit/report.hpp>

using namespace mk;
using namespace mk::report;

TEST_CASE("The constructor works correctly") {
    REQUIRE_NOTHROW(Report());
}

TEST_CASE("The open() method works correctly") {
    Report report;
    report.open([&](Error err) {
        REQUIRE(!err);
        report.open([&](Error err) {
            REQUIRE((err == ReportAlreadyOpen()));
        });
    });
}

TEST_CASE("The write_entry() method works correctly") {
    Report report;
    Entry entry;
    report.write_entry(entry, [&](Error err) {
        REQUIRE((err == ReportNotOpen()));
        report.open([&](Error err) {
            REQUIRE(!err);
            report.write_entry(entry, [&](Error err) {
                REQUIRE(!err);
                report.write_entry(entry, [&](Error err) {
                    REQUIRE(!err);
                    report.write_entry(entry, [&](Error err) {
                        REQUIRE(!err);
                        report.close([&](Error err) {
                            REQUIRE(!err);
                            report.write_entry(entry, [&](Error err) {
                                REQUIRE((err == ReportAlreadyClosed()));
                            });
                        });
                    });
                });
            });
        });
    });
}

TEST_CASE("The close() method works correctly") {
    Report report;
    report.open([&](Error err) {
        REQUIRE(!err);
        report.close([&](Error err) {
            REQUIRE(!err);
            report.close([&](Error err) {
                REQUIRE((err == ReportAlreadyClosed()));
            });
        });
    });
}
