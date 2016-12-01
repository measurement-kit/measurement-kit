// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "../src/libmeasurement_kit/ext/catch.hpp"

#include "../src/libmeasurement_kit/ooni/resources_impl.hpp"

using namespace mk;

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

static void get_no_loc(std::string, Callback<Error, Var<http::Response>> cb,
                       http::Headers, Settings, Var<Reactor>, Var<Logger>,
                       Var<http::Response>, int) {
    Var<http::Response> response{new http::Response};
    response->status_code = 300;
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

    SECTION("When the location header is missing") {
        ooni::resources::get_latest_release_impl<get_no_loc>(
            [=](Error e, std::string s) {
                REQUIRE(e.code == ooni::MissingLocationHeaderError().code);
                REQUIRE(s == "");
            },
            {}, Reactor::global(), Logger::global());
    }
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
