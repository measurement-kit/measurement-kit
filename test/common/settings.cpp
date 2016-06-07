// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <measurement_kit/common/settings.hpp>

using namespace mk;

TEST_CASE("The settings class convert string to int") {
    Settings settings;
    settings["key"] = "6";
    int value = settings["key"].as<int>();
    REQUIRE(value == 6);
}

TEST_CASE("The settings class throw an exception when the conversions fails, "
          "due to an empty string") {
    Settings settings;
    settings["key"] = "";
    REQUIRE_THROWS_AS(settings["key"].as<int>(), ValueError);
}

TEST_CASE("The settings class throw an exception when the conversion is not "
          "complete") {
    Settings settings;
    settings["key"] = "4 5 6";
    REQUIRE_THROWS_AS(settings["key"].as<int>(), ValueError);
}

TEST_CASE("The settings class convert bool to int") {
    Settings settings;
    settings["key"] = true;
    REQUIRE(settings["key"].as<int>() == 1);
}

TEST_CASE("The settings class convert bool to string") {
    Settings settings;
    settings["key"] = true;
    REQUIRE(settings["key"] == "1");
}

TEST_CASE("The settings class convert bool to double") {
    Settings settings;
    settings["key"] = true;
    REQUIRE(settings["key"].as<double>() == 1);
}

TEST_CASE(
    "The settings class throws an exception while converting double to int") {
    Settings settings;
    settings["key"] = 6.5;
    REQUIRE_THROWS_AS(settings["key"].as<int>(), ValueError);
}

TEST_CASE(
    "The settings class throws an exception when converting double to bool") {
    Settings settings;
    settings["key"] = 6.5;
    REQUIRE_THROWS_AS(settings["key"].as<bool>(), ValueError);
}

TEST_CASE("The settings class converts double to string") {
    Settings settings;
    settings["key"] = 6.5;
    REQUIRE(settings["key"] == "6.5");
}

TEST_CASE("The settings class converts int to double") {
    Settings settings;
    settings["key"] = 6;
    REQUIRE(settings["key"].as<double>() == 6.0);
}

TEST_CASE("The settings class converts int to string") {
    Settings settings;
    settings["key"] = 6;
    REQUIRE(settings["key"] == "6");
}

TEST_CASE(
    "The settings class throws an exception when converting int (10) to bool") {
    Settings settings;
    settings["key"] = 10;
    REQUIRE_THROWS_AS(settings["key"].as<bool>(), ValueError);
}

TEST_CASE("The settings class converts string to int") {
    Settings settings;
    settings["key"] = "6";
    REQUIRE(settings["key"].as<int>() == 6);
}

TEST_CASE("The settings class converts string to double") {
    Settings settings;
    settings["key"] = "6.7";
    REQUIRE(settings["key"].as<double>() == 6.7);
}

TEST_CASE("The settings class converts string '1' to bool") {
    Settings settings;
    settings["key"] = "1";
    REQUIRE(settings["key"].as<bool>() == true);
}

TEST_CASE("The settings class throws an exception when converting from string"
          "to bool with an uncorrect value") {
    Settings settings;
    settings["key"] = "10";
    REQUIRE_THROWS_AS(settings["key"].as<bool>(), ValueError);
}

TEST_CASE("The Settings::get() method works as expected "
          "when key does not exist") {
    Settings settings;
    REQUIRE(settings.get("key", 17.0) == 17.0);
    REQUIRE(settings.find("key") == settings.end());
}

TEST_CASE("The Settings::get() method works as expected when key exists") {
    Settings settings = {
        {"key", 21.0},
    };
    REQUIRE(settings.get("key", 17.0) == 21.0);
    REQUIRE(settings.find("key") != settings.end());
}


TEST_CASE("The Settings::get_noexcept() method works as expected "
          "when key does not exist") {
    Settings settings;
    ErrorOr<double> rv = settings.get_noexcept("key", 17.0);
    REQUIRE(*rv == 17.0);
    REQUIRE(settings.find("key") == settings.end());
}

TEST_CASE("The Settings::get_noexcept() method works as expected "
          "when key exists and conversion is possible") {
    Settings settings = {
        {"key", 21.0},
    };
    ErrorOr<double> rv = settings.get_noexcept("key", 17.0);
    REQUIRE(*rv == 21.0);
    REQUIRE(settings.find("key") != settings.end());
}

TEST_CASE("The Settings::get_noexcept() method works as expected "
          "when key exists and conversion is not possible") {
    Settings settings = {
        {"key", "xx"},
    };
    ErrorOr<double> rv = settings.get_noexcept("key", 17.0);
    REQUIRE(!rv);
    REQUIRE_THROWS_AS(*rv, Error);
    REQUIRE(settings.find("key") != settings.end());
}
