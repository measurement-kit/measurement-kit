// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "test/winsock.hpp"

#include "include/private/catch.hpp"

#include "src/libmeasurement_kit/common/error.hpp"

#include <string.h>

using namespace mk;

TEST_CASE("The default constructed error is true-ish") {
    Error err;
    REQUIRE(!err);
    REQUIRE((err.child_errors.size() <= 0));
    REQUIRE(err == 0);
    REQUIRE(err.reason == "");
}

TEST_CASE("Error constructed with error code is correctly initialized") {
    Error err{17};
    REQUIRE(!!err);
    REQUIRE((err.child_errors.size() <= 0));
    REQUIRE(err == 17);
    REQUIRE(err.reason == "unknown_failure 17");
}

TEST_CASE("Error constructed with error and message is correctly initialized") {
    Error err{17, "antani"};
    REQUIRE(!!err);
    REQUIRE((err.child_errors.size() <= 0));
    REQUIRE(err == 17);
    REQUIRE(err.reason == "antani");
}

TEST_CASE("Constructor with underlying error works correctly") {
    Error err{17, "antani", MockedError()};
    REQUIRE(!!err);
    REQUIRE(err.child_errors[0] == MockedError());
    REQUIRE(err == 17);
    REQUIRE(err.reason == "antani");
}

TEST_CASE("Equality works for errors") {
    Error first{17}, second{17};
    REQUIRE(first == second);
}

TEST_CASE("Unequality works for errors") {
    Error first{17}, second{21};
    REQUIRE(first != second);
}

MK_DEFINE_ERR(17, ExampleError, "example error")

TEST_CASE("The defined-error constructor with string works") {
    ExampleError ex{"antani"};
    REQUIRE(!!ex);
    REQUIRE(ex == 17);
    REQUIRE(ex.reason == "example error: antani");
    REQUIRE(strcmp(ex.what(), "example error: antani") == 0);
}

TEST_CASE("The add_child_error() method works") {
    Error err;
    ExampleError ex{"antani"};
    MockedError merr;
    err.add_child_error(std::move(ex));
    err.add_child_error(std::move(merr));
    REQUIRE((err.child_errors.size() == 2));
    REQUIRE((err.child_errors[0] == ExampleError()));
    REQUIRE((err.child_errors[0].reason == "example error: antani"));
    REQUIRE((err.child_errors[1] == MockedError()));
    REQUIRE((err.child_errors[1].reason == "mocked_error"));
}
