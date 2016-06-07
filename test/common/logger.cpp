// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <measurement_kit/common.hpp>

#include <string>

TEST_CASE("By default the logger is quiet") {
    std::string buffer;

    mk::on_log([&buffer](uint32_t, const char *s) {
        buffer += s;
        buffer += "\n";
    });

    mk::debug("Antani");
    mk::info("Foo");

    REQUIRE(buffer == "");
}

TEST_CASE("It is possible to make the logger verbose") {
    std::string buffer;

    mk::on_log([&buffer](uint32_t, const char *s) {
        buffer += s;
        buffer += "\n";
    });

    mk::set_verbosity(MK_LOG_INFO);

    mk::debug("Antani");
    mk::info("Foo");

    REQUIRE(buffer == "Foo\n");
}

TEST_CASE("Verbosity can be further increased") {
    std::string buffer;

    mk::on_log([&buffer](uint32_t, const char *s) {
        buffer += s;
        buffer += "\n";
    });

    mk::set_verbosity(MK_LOG_DEBUG);

    mk::debug("Antani");
    mk::info("Foo");

    REQUIRE(buffer == "Antani\nFoo\n");
}
