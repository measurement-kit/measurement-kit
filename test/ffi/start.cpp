// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "test/winsock.hpp"

#include "include/private/catch.hpp"

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
