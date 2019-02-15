// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "test/winsock.hpp"

#include "include/private/catch.hpp"

#include "src/libmeasurement_kit/common/settings.hpp"

using namespace mk;

TEST_CASE("The scalar class converts string to int") {
    Scalar scalar = "123456";
    REQUIRE(scalar.as<int>() == 123456);
}

TEST_CASE("The scalar class works even when it contains a space") {
    Scalar scalar = "123456 789";
    REQUIRE(scalar.as_string() == "123456 789");
}
