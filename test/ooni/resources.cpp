// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "../src/libmeasurement_kit/ext/catch.hpp"

#include "../src/libmeasurement_kit/ooni/resources_impl.hpp"

using namespace mk;

TEST_CASE("sanitize_version() works as expected") {
    REQUIRE(ooni::resources::sanitize_version("\t 1.2.3 \r\t\n  \r") ==
            "1.2.3");
}

static void get_fail(std::string, Callback<Error, Var<http::Response>> cb,
                     http::Headers, Settings, Var<Reactor>, Var<Logger>,
                     Var<http::Response>, int) {
    cb(MockedError(), nullptr);
}

static void get_500(std::string, Callback<Error, Var<http::Response>> cb,
                    http::Headers, Settings, Var<Reactor>, Var<Logger>,
                    Var<http::Response>, int) {
    Var<http::Response> response{new http::Response};
    response->status_code = 500;
    cb(NoError(), response);
}

TEST_CASE("get_latest_release() works as expected") {
    SECTION("When http::get() fails") {
        ooni::resources::get_latest_release_impl<get_fail>(
            [=](Error e, std::string s) {
                REQUIRE(e.code == MockedError().code);
                REQUIRE(s == "");
            },
            {}, Reactor::global(), Logger::global());
    }

    SECTION("When response is not a redirection") {
        ooni::resources::get_latest_release_impl<get_500>(
            [=](Error e, std::string s) {
                REQUIRE(e.code == ooni::CannotGetResourcesVersionError().code);
                REQUIRE(s == "");
            },
            {}, Reactor::global(), Logger::global());
    }

#if ENABLE_INTEGRATION_TESTS
    SECTION("Integration test") {
        Var<Reactor> reactor = Reactor::make();
        reactor->loop_with_initial_event([=]() {
            ooni::resources::get_latest_release(
                [=](Error e, std::string s) {
                    REQUIRE(e.code == NoError().code);
                    REQUIRE(s != "");
                    reactor->break_loop();
                },
                {}, reactor, Logger::global());
        });
    }
#endif
}

static void get_invalid_json(std::string,
                             Callback<Error, Var<http::Response>> cb,
                             http::Headers, Settings, Var<Reactor>, Var<Logger>,
                             Var<http::Response>, int) {
    Var<http::Response> response{new http::Response};
    response->status_code = 200;
    response->body = "{";
    cb(NoError(), response);
}

TEST_CASE("get_manifest_as_json() works as expected") {
    SECTION("When http::get() fails") {
        ooni::resources::get_manifest_as_json_impl<get_fail>(
            "2", [=](Error e, nlohmann::json s) {
                REQUIRE(e.code == MockedError().code);
                REQUIRE(s == nullptr);
            },
            {}, Reactor::global(), Logger::global());
    }

    SECTION("When response is not okay") {
        ooni::resources::get_manifest_as_json_impl<get_500>(
            "2", [=](Error e, nlohmann::json s) {
                REQUIRE(e.code == ooni::CannotGetResourcesManifestError().code);
                REQUIRE(s == nullptr);
            },
            {}, Reactor::global(), Logger::global());
    }

    SECTION("When the body is not a valid JSON") {
        ooni::resources::get_manifest_as_json_impl<get_invalid_json>(
            "2", [=](Error e, nlohmann::json s) {
                REQUIRE(e.code == JsonParseError().code);
                REQUIRE(s == nullptr);
            },
            {}, Reactor::global(), Logger::global());
    }
}

TEST_CASE("sanitize_path() works as expected") {
    // Important: let's also make sure that multiple sequences are stripped

    SECTION("When there are neither forward not back slashes") {
        REQUIRE(ooni::resources::sanitize_path("antani") == "antani");
    }

    SECTION("For backward slashes") {
        REQUIRE(ooni::resources::sanitize_path("/etc/passwd///")
                == ".etc.passwd.");
    }

    SECTION("For forward slashes") {
        REQUIRE(ooni::resources::sanitize_path("\\etc\\passwd\\\\\\")
                == ".etc.passwd.");
    }
}

static void get_antani_body(std::string,
                            Callback<Error, Var<http::Response>> cb,
                            http::Headers, Settings, Var<Reactor>, Var<Logger>,
                            Var<http::Response>, int) {
    Var<http::Response> response{new http::Response};
    response->status_code = 200;
    response->body = "antani";
    cb(NoError(), response);
}

static bool io_error(const std::ostream &) {
    return true;
}

TEST_CASE("get_resources_for_country() works as expected") {

    SECTION("When manifest is not an object") {
        Var<Reactor> reactor = Reactor::make();
        reactor->loop_with_initial_event([=]() {
            ooni::resources::get_resources_for_country_impl(
                "6", nullptr, "IT",
                [=](Error err) {
                    REQUIRE(err.code == JsonDomainError().code);
                    reactor->break_loop();
                },
                {}, reactor, Logger::global());
        });
    }

    SECTION("When manifest does not contain a resources section") {
        Var<Reactor> reactor = Reactor::make();
        reactor->loop_with_initial_event([=]() {
            ooni::resources::get_resources_for_country_impl(
                "6", nlohmann::json::object(), "IT",
                [=](Error err) {
                    REQUIRE(err.code == JsonKeyError().code);
                    reactor->break_loop();
                },
                {}, reactor, Logger::global());
        });
    }

    SECTION("When manifest resources are not objects") {
        Var<Reactor> reactor = Reactor::make();
        reactor->loop_with_initial_event([=]() {
            nlohmann::json root;
            root["resources"].push_back(nullptr);
            root["resources"].push_back(nullptr);
            root["resources"].push_back(nullptr);
            ooni::resources::get_resources_for_country_impl(
                "6", root, "IT",
                [=](Error err) {
                    REQUIRE(err.code == ParallelOperationError().code);
                    for (size_t i = 0; i < err.child_errors.size(); ++i) {
                        REQUIRE(err.child_errors[i]->code ==
                                JsonDomainError().code);
                    }
                    reactor->break_loop();
                },
                {}, reactor, Logger::global());
        });
    }

    SECTION("When manifest resources do not contain the country key") {
        Var<Reactor> reactor = Reactor::make();
        reactor->loop_with_initial_event([=]() {
            nlohmann::json root;
            root["resources"].push_back(nlohmann::json::object());
            root["resources"].push_back(nlohmann::json::object());
            root["resources"].push_back(nlohmann::json::object());
            ooni::resources::get_resources_for_country_impl(
                "6", root, "IT",
                [=](Error err) {
                    REQUIRE(err.code == ParallelOperationError().code);
                    for (size_t i = 0; i < err.child_errors.size(); ++i) {
                        REQUIRE(err.child_errors[i]->code ==
                                JsonKeyError().code);
                    }
                    reactor->break_loop();
                },
                {}, reactor, Logger::global());
        });
    }

    SECTION("When a specific country code is selected others are skipped") {
        Var<Reactor> reactor = Reactor::make();
        reactor->loop_with_initial_event([=]() {
            nlohmann::json root = R"({
                "resources": [{
                    "country_code": "IT"
                }, {
                    "country_code": "DE"
                }, {
                    "country_code": "FR"
                }
            ]})"_json;
            ooni::resources::get_resources_for_country(
                "6", root, "IT",
                [=](Error err) {
                    REQUIRE(err.code == ParallelOperationError().code);
                    /*
                     * JsonKeyError because `path` is missing.
                     */
                    REQUIRE(err.child_errors[0]->code == JsonKeyError().code);
                    REQUIRE(err.child_errors[1]->code == NoError().code);
                    REQUIRE(err.child_errors[2]->code == NoError().code);
                    reactor->break_loop();
                },
                {}, reactor, Logger::global());
        });
    }

    SECTION("The ALL selector selects all countries") {
        Var<Reactor> reactor = Reactor::make();
        reactor->loop_with_initial_event([=]() {
            nlohmann::json root = R"({
                "resources": [{
                    "country_code": "IT"
                }, {
                    "country_code": "DE"
                }, {
                    "country_code": "FR"
                }
            ]})"_json;
            ooni::resources::get_resources_for_country(
                "6", root, "ALL",
                [=](Error err) {
                    REQUIRE(err.code == ParallelOperationError().code);
                    /*
                     * JsonKeyError because `path` is missing.
                     */
                    REQUIRE(err.child_errors[0]->code == JsonKeyError().code);
                    REQUIRE(err.child_errors[1]->code == JsonKeyError().code);
                    REQUIRE(err.child_errors[2]->code == JsonKeyError().code);
                    reactor->break_loop();
                },
                {}, reactor, Logger::global());
        });
    }

    SECTION("Deals with HTTP GET errors") {
        Var<Reactor> reactor = Reactor::make();
        reactor->loop_with_initial_event([=]() {
            nlohmann::json root = R"({
                "resources": [{
                    "country_code": "IT",
                    "path": "xx"
                }, {
                    "country_code": "DE",
                    "path": "xx"
                }, {
                    "country_code": "FR",
                    "path": "xx"
                }
            ]})"_json;
            ooni::resources::get_resources_for_country_impl<get_fail>(
                "6", root, "ALL",
                [=](Error err) {
                    REQUIRE(err.code == ParallelOperationError().code);
                    REQUIRE(err.child_errors[0]->code == MockedError().code);
                    REQUIRE(err.child_errors[1]->code == MockedError().code);
                    REQUIRE(err.child_errors[2]->code == MockedError().code);
                    reactor->break_loop();
                },
                {}, reactor, Logger::global());
        });
    }

    SECTION("Deals with HTTP GET returning error") {
        Var<Reactor> reactor = Reactor::make();
        reactor->loop_with_initial_event([=]() {
            nlohmann::json root = R"({
                "resources": [{
                    "country_code": "IT",
                    "path": "xx"
                }, {
                    "country_code": "DE",
                    "path": "xx"
                }, {
                    "country_code": "FR",
                    "path": "xx"
                }
            ]})"_json;
            ooni::resources::get_resources_for_country_impl<get_500>(
                "6", root, "ALL",
                [=](Error err) {
                    REQUIRE(err.code == ParallelOperationError().code);
                    for (size_t i = 0; i < err.child_errors.size(); ++i) {
                        REQUIRE(err.child_errors[i]->code ==
                                ooni::CannotGetResourceError().code);
                    }
                    reactor->break_loop();
                },
                {}, reactor, Logger::global());
        });
    }

    SECTION("Deals with missing sha256 keys in the manifest") {
        Var<Reactor> reactor = Reactor::make();
        reactor->loop_with_initial_event([=]() {
            nlohmann::json root = R"({
                "resources": [{
                    "country_code": "IT",
                    "path": "xx"
                }, {
                    "country_code": "DE",
                    "path": "xx"
                }, {
                    "country_code": "FR",
                    "path": "xx"
                }
            ]})"_json;
            ooni::resources::get_resources_for_country_impl<get_antani_body>(
                "6", root, "ALL",
                [=](Error err) {
                    REQUIRE(err.code == ParallelOperationError().code);
                    for (size_t i = 0; i < err.child_errors.size(); ++i) {
                        REQUIRE(err.child_errors[i]->code ==
                                JsonKeyError().code);
                    }
                    reactor->break_loop();
                },
                {}, reactor, Logger::global());
        });
    }

    SECTION("Deals with invalid sha256 sums") {
        Var<Reactor> reactor = Reactor::make();
        reactor->loop_with_initial_event([=]() {
            nlohmann::json root = R"({
                "resources": [{
                    "country_code": "IT",
                    "path": "xx",
                    "sha256": "abc"
                }, {
                    "country_code": "DE",
                    "path": "xx",
                    "sha256": "abc"
                }, {
                    "country_code": "FR",
                    "path": "xx",
                    "sha256": "abc"
                }
            ]})"_json;
            ooni::resources::get_resources_for_country_impl<get_antani_body>(
                "6", root, "ALL",
                [=](Error err) {
                    REQUIRE(err.code == ParallelOperationError().code);
                    for (size_t i = 0; i < err.child_errors.size(); ++i) {
                        REQUIRE(err.child_errors[i]->code ==
                                ooni::ResourceIntegrityError().code);
                    }
                    reactor->break_loop();
                },
                {}, reactor, Logger::global());
        });
    }

    SECTION("Deals with write file I/O error") {
        Var<Reactor> reactor = Reactor::make();
        reactor->loop_with_initial_event([=]() {
            nlohmann::json root = R"({
                "resources": [{
                    "country_code": "IT",
                    "path": "xx",
                    "sha256": "b1dc5f0ba862fe3a1608d985ded3c5ed6b9a7418db186d9e6e6201794f59ba54"
                }, {
                    "country_code": "DE",
                    "path": "xx",
                    "sha256": "b1dc5f0ba862fe3a1608d985ded3c5ed6b9a7418db186d9e6e6201794f59ba54"
                }, {
                    "country_code": "FR",
                    "path": "xx",
                    "sha256": "b1dc5f0ba862fe3a1608d985ded3c5ed6b9a7418db186d9e6e6201794f59ba54"
                }
            ]})"_json;
            ooni::resources::get_resources_for_country_impl<get_antani_body,
                                                            io_error>(
                "6", root, "ALL",
                [=](Error err) {
                    REQUIRE(err.code == ParallelOperationError().code);
                    for (size_t i = 0; i < err.child_errors.size(); ++i) {
                        REQUIRE(err.child_errors[i]->code ==
                                FileIoError().code);
                    }
                    reactor->break_loop();
                },
                {}, reactor, Logger::global());
        });
    }
}

static void get_manifest_as_json_fail(std::string,
                                      Callback<Error, nlohmann::json> callback,
                                      Settings, Var<Reactor>, Var<Logger>) {
    callback(MockedError(), nullptr);
}

static void get_manifest_as_json_okay(std::string,
                                      Callback<Error, nlohmann::json> callback,
                                      Settings, Var<Reactor>, Var<Logger>) {
    callback(NoError(), nullptr);
}

static void get_resources_for_country_fail(std::string, nlohmann::json,
                                           std::string,
                                           Callback<Error> callback, Settings,
                                           Var<Reactor>, Var<Logger>) {
    callback(MockedError());
}

TEST_CASE("get_resources() works as expected") {

    SECTION("When get_manifest_as_json() fails") {
        Var<Reactor> reactor = Reactor::make();
        reactor->loop_with_initial_event([=]() {
            ooni::resources::get_resources_impl<get_manifest_as_json_fail>(
                "6", "IT",
                [=](Error error) {
                    REQUIRE(error.code == MockedError().code);
                    reactor->break_loop();
                },
                {}, reactor, Logger::global());
        });
    }

    SECTION("When get_resources_for_country() fails") {
        Var<Reactor> reactor = Reactor::make();
        reactor->loop_with_initial_event([=]() {
            ooni::resources::get_resources_impl<get_manifest_as_json_okay,
                                                get_resources_for_country_fail>(
                "6", "IT",
                [=](Error error) {
                    REQUIRE(error.code == MockedError().code);
                    reactor->break_loop();
                },
                {}, reactor, Logger::global());
        });
    }

#ifdef ENABLE_INTEGRATION_TESTS
    SECTION("Integration test") {
        Var<Reactor> reactor = Reactor::make();
        Var<Logger> logger = Logger::global();
        logger->set_verbosity(MK_LOG_INFO);
        reactor->loop_with_initial_event([=]() {
            ooni::resources::get_resources("6", "ALL",
                                           [=](Error error) {
                                               REQUIRE(error.code ==
                                                       NoError().code);
                                               reactor->break_loop();
                                           },
                                           {}, reactor, logger);
        });
    }
#endif
}
