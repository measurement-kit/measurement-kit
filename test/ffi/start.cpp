// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/libmeasurement_kit/ext/catch.hpp"

#include <measurement_kit/ffi.h>

TEST_CASE("mk_task_start() works as expected") {
    SECTION("With nullptr settings") {
        auto task = mk_task_start(nullptr);
        REQUIRE(task == nullptr);
    }

    SECTION("With invalid settings") {
        auto task = mk_task_start("{");
        REQUIRE(task == nullptr);
    }
}

TEST_CASE("mk_task_start_ex() works as expected") {
    SECTION("With nullptr task pointer") {
        auto err = mk_task_start_ex(nullptr, "{}");
        REQUIRE(err == MK_TASK_EGENERIC);
    }

    SECTION("With nullptr settings") {
        mk_task_t *task = (mk_task_t *)0x1234;
        auto err = mk_task_start_ex(&task, nullptr);
        REQUIRE(err == MK_TASK_EGENERIC);
        REQUIRE(task == nullptr);
    }

    SECTION("With invalid settings") {
        mk_task_t *task = (mk_task_t *)0x1234;
        auto err = mk_task_start_ex(&task, "{");
        REQUIRE(err == MK_TASK_EPARSE);
        REQUIRE(task == nullptr);
    }
}
