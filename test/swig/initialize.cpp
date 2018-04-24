// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "test/winsock.hpp"

#define CATCH_CONFIG_MAIN
#include "src/libmeasurement_kit/ext/catch.hpp"

#define MK_EXPOSE_SWIG_API
#include <measurement_kit/swig.hpp>

TEST_CASE("initialize() works as expected") {
    SECTION("With invalid settings") {
        mk::swig::Task task;
        REQUIRE(task.initialize("{") == false);
    }

    SECTION("When called multiple times") {
        mk::swig::Task task;
        REQUIRE(task.initialize("{}") == true);
        REQUIRE(task.initialize("{}") == false);
    }
}

TEST_CASE("initialize_ex() works as expected") {
    SECTION("With invalid settings") {
        mk::swig::Task task;
        auto rv = task.initialize_ex("{");
        REQUIRE(rv.result == false);
        REQUIRE(rv.reason == "parse error");
    }

    SECTION("When called multiple times") {
        mk::swig::Task task;
        auto rv1 = task.initialize_ex("{}");
        REQUIRE(rv1.result == true);
        REQUIRE(rv1.reason == "");
        auto rv2 = task.initialize_ex("{}");
        REQUIRE(rv2.result == false);
        REQUIRE(rv2.reason == "already initialized");
    }
}
