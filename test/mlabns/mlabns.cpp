// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/libmeasurement_kit/ext/catch.hpp"

#include "src/libmeasurement_kit/mlabns/mlabns_impl.hpp"

using namespace mk;

#ifdef ENABLE_INTEGRATION_TESTS

TEST_CASE("Query works as expected") {
    Settings settings;
    settings["mlabns/address_family"] = "ipv4";
    settings["mlabns/metro"] = "trn";
    settings["mlabns/policy"] = "random";
    std::string tool = "neubot";

    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        mlabns::query(tool,
                      [=](Error error, mlabns::Reply) {
                          REQUIRE(!error);
                          reactor->stop();
                      },
                      settings, reactor);
    });
}

TEST_CASE("Query can pass the settings to the dns level") {
    Settings settings;
    settings["mlabns/address_family"] = "ipv4";
    settings["mlabns/metro"] = "trn";
    settings["mlabns/policy"] = "random";
    settings["dns/nameserver"] = "8.8.8.1";
    settings["dns/engine"] = "libevent";
    std::string tool = "neubot";

    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        mlabns::query(tool,
                      [=](Error error, mlabns::Reply) {
                          REQUIRE(error);
                          reactor->stop();
                      },
                      settings, reactor);
    });
}

#endif

TEST_CASE("Make sure that an error is passed to callback with invalid "
          "address_family settings") {
    Settings settings;
    settings["mlabns/address_family"] = "ip4"; // Invalid
    settings["mlabns/metro"] = "trn";
    settings["mlabns/policy"] = "random";
    std::string tool = "neubot";
    SharedPtr<Reactor> reactor = Reactor::make();

    reactor->run_with_initial_event([=]() {
        mlabns::query(tool,
                      [=](Error error, mlabns::Reply) {
                          REQUIRE(error);
                          reactor->stop();
                      },
                      settings, reactor);
    });
}

TEST_CASE("Make sure that an error is passed to callback with invalid metro "
          "settings") {
    Settings settings;
    settings["mlabns/address_family"] = "ipv4";
    settings["mlabns/metro"] = "trno"; // Invalid
    settings["mlabns/policy"] = "random";
    std::string tool = "neubot";

    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        mlabns::query(tool,
                      [=](Error error, mlabns::Reply) {
                          REQUIRE(error);
                          reactor->stop();
                      },
                      settings, reactor);
    });
}

TEST_CASE("Make sure that an error is passed to callback with invalid policy "
          "settings") {
    Settings settings;
    settings["mlabns/address_family"] = "ipv4";
    settings["mlabns/metro"] = "trn";
    settings["mlabns/policy"] = "antani"; // Invalid
    std::string tool = "neubot";

    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        mlabns::query(tool,
                      [=](Error error, mlabns::Reply) {
                          REQUIRE(error);
                          reactor->stop();
                      },
                      settings, reactor);
    });
}

TEST_CASE("Make sure that an error is passed to callback with invalid tool "
          "settings") {
    Settings settings;
    settings["mlabns/address_family"] = "ipv4";
    settings["mlabns/metro"] = "trn";
    settings["mlabns/policy"] = "random";
    std::string tool = "antani"; // Invalid

    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        mlabns::query(tool,
                      [=](Error error, mlabns::Reply) {
                          REQUIRE(error);
                          reactor->stop();
                      },
                      settings, reactor);
    });
}

static void
get_debug_error(std::string, std::string, http::Headers,
                Callback<Error, SharedPtr<http::Response>, Json> cb,
                Settings, SharedPtr<Reactor>, SharedPtr<Logger>) {
    cb(MockedError(), SharedPtr<http::Response>::make(), {});
}

TEST_CASE(
    "Make sure that an error is passed to callback if http::request fails") {
    Settings settings;
    settings["mlabns/address_family"] = "ipv4";
    settings["mlabns/metro"] = "trn";
    settings["mlabns/policy"] = "random";
    std::string tool = "neubot";

    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        mlabns::query_impl<get_debug_error>(tool,
                                            [=](Error error, mlabns::Reply) {
                                                REQUIRE(error == MockedError());
                                                reactor->stop();
                                            },
                                            settings,
                                            reactor,
                                            Logger::global());
    });
}

static void get_debug_invalid_incomplete_json(
      std::string, std::string, http::Headers,
      Callback<Error, SharedPtr<http::Response>, Json> cb, Settings,
      SharedPtr<Reactor>, SharedPtr<Logger>) {
    SharedPtr<http::Response> response = SharedPtr<http::Response>::make();
    response->status_code = 200;
    // This json does not contain the country field
    response->body = "{\"city\": \"Turin\", \"url\": "
                     "\"http://"
                     "neubot.mlab.mlab1.trn01.measurement-lab.org:8080\", "
                     "\"ip\": [\"194.116.85.211\"], \"fqdn\": "
                     "\"neubot.mlab.mlab1.trn01.measurement-lab.org\", "
                     "\"site\": \"trn01\"}";
    cb(NoError(), response, Json::parse(response->body));
}

TEST_CASE("Make sure that an error is passed to callback if the response does "
          "not contain all fields") {
    Settings settings;
    settings["mlabns/address_family"] = "ipv4";
    settings["mlabns/metro"] = "trn";
    settings["mlabns/policy"] = "random";
    std::string tool = "neubot";

    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        mlabns::query_impl<get_debug_invalid_incomplete_json>(
            tool,
            [=](Error error, mlabns::Reply) {
                REQUIRE(error == JsonKeyError());
                reactor->stop();
            },
            settings, reactor, Logger::global());
    });
}

static void get_debug_json_with_unexpected_type(
      std::string, std::string, http::Headers,
      Callback<Error, SharedPtr<http::Response>, Json> cb, Settings,
      SharedPtr<Reactor>, SharedPtr<Logger>) {
    SharedPtr<http::Response> response = SharedPtr<http::Response>::make();
    response->status_code = 200;
    // IP is a int rather than being a list
    response->body = "{\"city\": \"Turin\", \"url\": "
                     "\"http://"
                     "neubot.mlab.mlab1.trn01.measurement-lab.org:8080\", "
                     "\"ip\": 194, \"fqdn\": "
                     "\"neubot.mlab.mlab1.trn01.measurement-lab.org\", "
                     "\"site\": \"trn01\", \"country\": \"IT\"}";
    cb(NoError(), response, {});
}

TEST_CASE("Make sure that an error is passed to callback if the response "
          "contains a field with invalid type") {
    Settings settings;
    settings["mlabns/address_family"] = "ipv4";
    settings["mlabns/metro"] = "trn";
    settings["mlabns/policy"] = "random";
    std::string tool = "neubot";

    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {
        mlabns::query_impl<get_debug_json_with_unexpected_type>(
            tool,
            [=](Error error, mlabns::Reply) {
                REQUIRE(error == JsonDomainError());
                reactor->stop();
            },
            settings, reactor, Logger::global());
    });
}
