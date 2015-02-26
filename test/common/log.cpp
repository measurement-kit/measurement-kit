/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

//
// Tests for src/common/log.cpp
//

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include "common/log.hpp"

#include <string>

TEST_CASE("By default the logger is quiet") {
    std::string buffer;

    ight_set_logger([&buffer](const char *s) {
        buffer += s;
        buffer += "\n";
    });

    ight_debug("Antani");
    ight_info("Foo");

    REQUIRE(buffer == "");
}

TEST_CASE("It is possible to make the logger verbose") {
    std::string buffer;

    ight_set_logger([&buffer](const char *s) {
        buffer += s;
        buffer += "\n";
    });

    ight_set_verbose(1);

    ight_debug("Antani");
    ight_info("Foo");

    REQUIRE(buffer == "Antani\nFoo\n");
}
