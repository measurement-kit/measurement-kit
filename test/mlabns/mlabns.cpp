// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include "src/mlabns/mlabns_impl.hpp"
#include <measurement_kit/common.hpp>
#include <measurement_kit/http.hpp>
#include <measurement_kit/mlabns.hpp>

using namespace mk;

TEST_CASE("Query works as expected") {
    Settings settings;
    settings["mlabns/address_family"] = "ipv4";
    settings["mlabns/metro"] = "trn";
    settings["mlabns/policy"] = "random";
    std::string tool = "neubot";

    loop_with_initial_event([=]() {
        mlabns::query(tool,
                      [](Error error, mlabns::Reply) {
                          REQUIRE(!error);
                          break_loop();
                      },
                      settings);
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
        mlabns::query(tool,
                      [](Error error, mlabns::Reply) {
                          REQUIRE(error);
                          break_loop();
                      },
                      settings);
    });
}

TEST_CASE("Make sure that an error is passed to callback with invalid "
          "address_family settings") {
    Settings settings;
    settings["mlabns/address_family"] = "ip4"; // Invalid
    settings["mlabns/metro"] = "trn";
    settings["mlabns/policy"] = "random";
    std::string tool = "neubot";

    loop_with_initial_event([=]() {
        mlabns::query(tool,
                      [](Error error, mlabns::Reply) {
                          REQUIRE(error);
                          break_loop();
                      },
                      settings);
    });
}

TEST_CASE("Make sure that an error is passed to callback with invalid metro "
          "settings") {
    Settings settings;
    settings["mlabns/address_family"] = "ipv4";
    settings["mlabns/metro"] = "trno"; // Invalid
    settings["mlabns/policy"] = "random";
    std::string tool = "neubot";

    loop_with_initial_event([=]() {
        mlabns::query(tool,
                      [](Error error, mlabns::Reply) {
                          REQUIRE(error);
                          break_loop();
                      },
                      settings);
    });
}

TEST_CASE("Make sure that an error is passed to callback with invalid policy "
          "settings") {
    Settings settings;
    settings["mlabns/address_family"] = "ipv4";
    settings["mlabns/metro"] = "trn";
    settings["mlabns/policy"] = "antani"; // Invalid
    std::string tool = "neubot";

    loop_with_initial_event([=]() {
        mlabns::query(tool,
                      [](Error error, mlabns::Reply) {
                          REQUIRE(error);
                          break_loop();
                      },
                      settings);
    });
}

TEST_CASE("Make sure that an error is passed to callback with invalid tool "
          "settings") {
    Settings settings;
    settings["mlabns/address_family"] = "ipv4";
    settings["mlabns/metro"] = "trn";
    settings["mlabns/policy"] = "random";
    std::string tool = "antani"; // Invalid

    loop_with_initial_event([=]() {
        mlabns::query(tool,
                      [](Error error, mlabns::Reply) {
                          REQUIRE(error);
                          break_loop();
                      },
                      settings);
    });
}

static void get_debug_error(std::string, Callback<Error, Var<http::Response>> cb,
                            http::Headers, Settings, Var<Reactor>,
                            Var<Logger>) {
    cb(MockedError(), nullptr);
}

TEST_CASE(
    "Make sure that an error is passed to callback if http::request fails") {
    Settings settings;
    settings["mlabns/address_family"] = "ipv4";
    settings["mlabns/metro"] = "trn";
    settings["mlabns/policy"] = "random";
    std::string tool = "neubot";

    loop_with_initial_event([=]() {
        mlabns::query_impl<get_debug_error>(tool,
                                            [](Error error, mlabns::Reply) {
                                                REQUIRE(error == MockedError());
                                                break_loop();
                                            },
                                            settings,
                                            Reactor::global(),
                                            Logger::global());
    });
}

static void get_debug_invalid_status_code(std::string, Callback<Error, Var<http::Response>> cb,
                                          http::Headers, Settings,
                                          Var<Reactor>, Var<Logger>) {
    Var<http::Response> response(new http::Response);
    response->status_code = 500;
    cb(NoError(), response);
}

TEST_CASE("Make sure that an error is passed to callback if the response "
          "status is not 200") {
    Settings settings;
    settings["mlabns/address_family"] = "ipv4";
    settings["mlabns/metro"] = "trn";
    settings["mlabns/policy"] = "random";
    std::string tool = "neubot";

    loop_with_initial_event([=]() {
        mlabns::query_impl<get_debug_invalid_status_code>(
            tool,
            [](Error error, mlabns::Reply) {
                REQUIRE(error == http::HttpRequestFailedError());
                break_loop();
            },
            settings, Reactor::global(), Logger::global());
    });
}

static void get_debug_invalid_response(std::string, Callback<Error, Var<http::Response>> cb,
                                       http::Headers, Settings,
                                       Var<Reactor>, Var<Logger>) {
    Var<http::Response> response(new http::Response);
    response->status_code = 200;
    response->body = "alfj9882//234j<<<384982";
    cb(NoError(), response);
}

TEST_CASE("Make sure that an error is passed to callback if the response is "
          "not a json") {
    Settings settings;
    settings["mlabns/address_family"] = "ipv4";
    settings["mlabns/metro"] = "trn";
    settings["mlabns/policy"] = "random";
    std::string tool = "neubot";

    loop_with_initial_event([=]() {
        mlabns::query_impl<get_debug_invalid_response>(
            tool,
            [](Error error, mlabns::Reply) {
                REQUIRE(error == JsonParseError());
                break_loop();
            },
            settings, Reactor::global(), Logger::global());
    });
}

static void get_debug_invalid_uncomplete_json(std::string,
                                              Callback<Error, Var<http::Response>> cb,
                                              http::Headers,
                                              Settings, Var<Reactor>,
                                              Var<Logger>) {
    Var<http::Response> response(new http::Response);
    response->status_code = 200;
    // This json does not contain the country field
    response->body = "{\"city\": \"Turin\", \"url\": "
                    "\"http://"
                    "neubot.mlab.mlab1v4.trn01.measurement-lab.org:8080\", "
                    "\"ip\": [\"194.116.85.211\"], \"fqdn\": "
                    "\"neubot.mlab.mlab1v4.trn01.measurement-lab.org\", "
                    "\"site\": \"trn01\"}";
    cb(NoError(), response);
}

TEST_CASE("Make sure that an error is passed to callback if the response does "
          "not contain all fields") {
    Settings settings;
    settings["mlabns/address_family"] = "ipv4";
    settings["mlabns/metro"] = "trn";
    settings["mlabns/policy"] = "random";
    std::string tool = "neubot";

    loop_with_initial_event([=]() {
        mlabns::query_impl<get_debug_invalid_uncomplete_json>(
            tool,
            [](Error error, mlabns::Reply) {
                REQUIRE(error == JsonKeyError());
                break_loop();
            },
            settings, Reactor::global(), Logger::global());
    });
}

static void get_debug_json_with_unexpected_type(std::string,
                                                Callback<Error, Var<http::Response>> cb,
                                                http::Headers,
                                                Settings, Var<Reactor>,
                                                Var<Logger>) {
    Var<http::Response> response(new http::Response);
    response->status_code = 200;
    // IP is a int rather than being a list
    response->body = "{\"city\": \"Turin\", \"url\": "
                    "\"http://"
                    "neubot.mlab.mlab1v4.trn01.measurement-lab.org:8080\", "
                    "\"ip\": 194, \"fqdn\": "
                    "\"neubot.mlab.mlab1v4.trn01.measurement-lab.org\", "
                    "\"site\": \"trn01\", \"country\": \"IT\"}";
    cb(NoError(), response);
}

TEST_CASE("Make sure that an error is passed to callback if the response "
          "contains a field with invalid type") {
    Settings settings;
    settings["mlabns/address_family"] = "ipv4";
    settings["mlabns/metro"] = "trn";
    settings["mlabns/policy"] = "random";
    std::string tool = "neubot";

    loop_with_initial_event([=]() {
        mlabns::query_impl<get_debug_json_with_unexpected_type>(
            tool,
            [](Error error, mlabns::Reply) {
                REQUIRE(error == JsonDomainError());
                break_loop();
            },
            settings, Reactor::global(), Logger::global());
    });
}
