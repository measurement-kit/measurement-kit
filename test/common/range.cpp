// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "private/ext/catch.hpp"

#include "private/common/range.hpp"

#include <measurement_kit/common.hpp>

TEST_CASE("The mk::range() API works as expected") {
    std::vector<int> v = mk::range<int>(16);
    REQUIRE((v == std::vector<int>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
                                   13, 14, 15}));
}
