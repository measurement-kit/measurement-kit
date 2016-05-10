// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <measurement_kit/common.hpp>
#include <measurement_kit/mlabns.hpp>

using namespace mk;

TEST_CASE("Query works as expected") {
    Settings settings;
    settings["mlabns/address_family"] = "ipv4";
    settings["mlabns/metro"] = "trn";
    settings["mlabns/policy"] = "random";
    std::string tool = "neubot";

    loop_with_initial_event([=]() {
        mlabns::query(
            tool, [](Error error, mlabns::Reply reply) {
                REQUIRE(!error);
                break_loop();
            }, settings);
    });
}

TEST_CASE("Query can pass the settings to the dns level") {
    Settings settings;
    settings["mlabns/address_family"] = "ipv4";
    settings["mlabns/metro"] = "trn";
    settings["mlabns/policy"] = "random";
    settings["dns/nameserver"] = "8.8.8.1";
    std::string tool = "neubot";

    loop_with_initial_event([=]() {
        mlabns::query(
            tool, [](Error error, mlabns::Reply reply) {
                REQUIRE(error);
                break_loop();
            }, settings);
    });


}
