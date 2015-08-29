// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

//
// Tests for src/common/log.cpp
//

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <measurement_kit/common.hpp>

#include <string>

TEST_CASE("By default the logger is quiet") {
    std::string buffer;

    measurement_kit::on_log([&buffer](const char *s) {
        buffer += s;
        buffer += "\n";
    });

    measurement_kit::debug("Antani");
    measurement_kit::info("Foo");

    REQUIRE(buffer == "");
}

TEST_CASE("It is possible to make the logger verbose") {
    std::string buffer;

    measurement_kit::on_log([&buffer](const char *s) {
        buffer += s;
        buffer += "\n";
    });

    measurement_kit::set_verbose(1);

    measurement_kit::debug("Antani");
    measurement_kit::info("Foo");

    REQUIRE(buffer == "Antani\nFoo\n");
}
