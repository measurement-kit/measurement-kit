// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN

#include "src/ext/Catch/single_include/catch.hpp"
#include <measurement_kit/report.hpp>

using namespace mk::report;
using namespace mk;

TEST_CASE("By default we have an empty entry") {
    Entry entry;
    REQUIRE(entry.dump() == "null");
}

TEST_CASE("We can add key, value pairs to an empty entry") {
    Entry entry;
    entry["foo"] = "foobar";
    entry["foobar"] = 1.3;
    entry["baz"] = nullptr;
    entry["jarjar"] = false;
    REQUIRE(
        entry.dump() ==
        "{\"baz\":null,\"foo\":\"foobar\",\"foobar\":1.3,\"jarjar\":false}");
}

TEST_CASE("We can append elements to an empty entry") {
    Entry entry;
    entry.push_back("foobar");
    entry.push_back(1.3);
    entry.push_back(nullptr);
    entry.push_back(false);
    REQUIRE(entry.dump() == "[\"foobar\",1.3,null,false]");
}

TEST_CASE("We can add objects and lists to an existing entry") {
    Entry entry;
    entry["dict"]["foo"] = "foobar";
    entry["dict"]["foobar"] = 1.3;
    entry["dict"]["baz"] = nullptr;
    entry["dict"]["jarjar"] = false;
    entry["list"].push_back("foobar");
    entry["list"].push_back(1.3);
    entry["list"].push_back(nullptr);
    entry["list"].push_back(false);
    REQUIRE(entry.dump() == "{\"dict\":{\"baz\":null,\"foo\":\"foobar\","
                            "\"foobar\":1.3,\"jarjar\":false},\"list\":["
                            "\"foobar\",1.3,null,false]}");
}

TEST_CASE("We raise mk::JsonDomainError when keys cannot be added to entry") {
    Entry entry;
    // Operate on entry["list"] rather than on Entry to make sure that after
    // more than one access cycle we're still using the Entry type, which raises
    // JsonDomainError, rather than the base type nlohmann/json.
    entry["list"].push_back(17.0);
    REQUIRE_THROWS_AS((entry["list"]["foo"] = "foobar"), JsonDomainError);
}

TEST_CASE("We raise mk::JsonDomainError when we cannot append added to entry") {
    Entry entry;
    // Operate on entry["dict"] rather than on Entry to make sure that after
    // more than one access cycle we're still using the Entry type, which raises
    // JsonDomainError, rather than the base type nlohmann/json.
    entry["dict"]["foo"] = "foobar";
    REQUIRE_THROWS_AS((entry["dict"].push_back(17.0)), JsonDomainError);
}

TEST_CASE("We can create an Array") {
    Entry entry = Entry::array();
    entry.push_back(10.0);
    entry.push_back(9.0);
    entry.push_back(11.0);
    entry.push_back(8.0);
    REQUIRE(entry.dump() == "[10.0,9.0,11.0,8.0]");
}

TEST_CASE("Cast works when possible") {
    Entry entry{{"foo", "bar"}, {"baz", 17.0}};
    ErrorOr<std::string> s = entry["foo"];
    REQUIRE(!!s);
    REQUIRE(*s == "bar");
}

TEST_CASE("Cast returns error when not possible") {
    Entry entry{{"foo", nullptr}, {"baz", 17.0}};
    ErrorOr<std::string> s = entry["foo"];
    REQUIRE(!s);
    REQUIRE(s.as_error() == JsonDomainError());
}

TEST_CASE("Make sure it does not throw when accessing nonexistent key") {
    Entry entry{{"foo", "bar"}, {"baz", 17.0}};
    REQUIRE((entry["bar"] == nullptr));
}
