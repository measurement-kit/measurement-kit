// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "private/ext/catch.hpp"

#include <measurement_kit/common.hpp>

using namespace mk;

TEST_CASE("The default constructed error is true-ish") {
    Error err;
    REQUIRE(!err);
    REQUIRE((err.child_errors.size() <= 0));
    REQUIRE(err.code == 0);
    REQUIRE(err.reason == "");
}

TEST_CASE("Error constructed with error code is correctly initialized") {
    Error err{17};
    REQUIRE(!!err);
    REQUIRE((err.child_errors.size() <= 0));
    REQUIRE(err.code == 17);
    REQUIRE(err.reason == "unknown_failure 17");
}

TEST_CASE("Error constructed with error and message is correctly initialized") {
    Error err{17, "antani"};
    REQUIRE(!!err);
    REQUIRE((err.child_errors.size() <= 0));
    REQUIRE(err.code == 17);
    REQUIRE(err.reason == "antani");
}

TEST_CASE("Constructor with underlying error works correctly") {
    Error err{17, "antani", MockedError()};
    REQUIRE(!!err);
    REQUIRE(*err.child_errors[0] == MockedError());
    REQUIRE(err.code == 17);
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
    REQUIRE(ex.code == 17);
    REQUIRE(ex.as_ooni_error() == "example error: antani");
    REQUIRE(ex.reason == "example error: antani");
}

TEST_CASE("The add_child_error() method works") {
    Error err;
    ExampleError ex{"antani"};
    MockedError merr;
    err.add_child_error(ex);
    err.add_child_error(merr);
    REQUIRE((err.child_errors.size() == 2));
    REQUIRE((err.child_errors[0]->code == ExampleError().code));
    REQUIRE((err.child_errors[0]->reason == "example error: antani"));
    REQUIRE((err.child_errors[1]->code == MockedError().code));
    REQUIRE((err.child_errors[1]->reason == "mocked_error"));
}
