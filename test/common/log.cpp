// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

//
// Tests for src/common/log.cpp
//

#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include <measurement_kit/common.hpp>

#include <string>

TEST_CASE("By default the logger is quiet") {
    std::string buffer;

    mk::on_log([&buffer](const char *s) {
        buffer += s;
        buffer += "\n";
    });

    mk::debug("Antani");
    mk::info("Foo");

    REQUIRE(buffer == "");
}

TEST_CASE("It is possible to make the logger verbose") {
    std::string buffer;

    mk::on_log([&buffer](const char *s) {
        buffer += s;
        buffer += "\n";
    });

    mk::set_verbose(1);

    mk::debug("Antani");
    mk::info("Foo");

    REQUIRE(buffer == "Antani\nFoo\n");
}
