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

TEST_CASE("We raise mk::DomainError when keys cannot be added to entry") {
    Entry entry;
    entry.push_back(17.0);
    REQUIRE_THROWS_AS((entry["foo"] = "foobar"), DomainError);
}

TEST_CASE("We raise mk::DomainError when we cannot append added to entry") {
    Entry entry;
    entry["foo"] = "foobar";
    REQUIRE_THROWS_AS((entry.push_back(17.0)), DomainError);
}
