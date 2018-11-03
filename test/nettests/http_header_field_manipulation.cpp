// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "test/winsock.hpp"

#include "src/libmeasurement_kit/ext/catch.hpp"
#include "src/libmeasurement_kit/ooni/http_header_field_manipulation.hpp"

#include "utils.hpp"

using namespace mk::nettests;
using namespace mk;

#ifdef ENABLE_INTEGRATION_TESTS

TEST_CASE("Synchronous http-header-field-manipulation test") {
    test::nettests::with_test<HttpHeaderFieldManipulationTest>(
          test::nettests::run_test);
}

#endif

TEST_CASE("compare_headers_response works") {
    SharedPtr<report::Entry> entry(new report::Entry);
    (*entry)["tampering"] = report::Entry::object();

    SharedPtr<http::Response> response{new http::Response};

    SECTION("For empty response") {
        ooni::compare_headers_response({ {"foo", "bar"} },
                                       response,
                                       entry,
                                        Logger::global());
        REQUIRE((*entry)["tampering"]["total"] == true);
        REQUIRE((*entry)["tampering"]["request_line_capitalization"] == true);
    }
    SECTION("For invalid JSON") {
        response->body = "<html></html>";
        ooni::compare_headers_response({ {"foo", "bar"} },
                                       response,
                                       entry,
                                       Logger::global());
        REQUIRE((*entry)["tampering"]["total"] == true);
        REQUIRE((*entry)["tampering"]["request_line_capitalization"] == true);
    }
    SECTION("For missing request line") {
        response->body = "{\"error\": \"blocked\"}";
        ooni::compare_headers_response({ {"foo", "bar"} },
                                       response,
                                       entry,
                                       Logger::global());
        REQUIRE((*entry)["tampering"]["total"].get<bool>() == false);
        REQUIRE((*entry)["tampering"]["request_line_capitalization"] == true);
    }
    SECTION("For altered request line") {
        response->body = "{\"request_line\": \"get / http/1.1\"}";
        ooni::compare_headers_response({ {"foo", "bar"} },
                                       response,
                                       entry,
                                       Logger::global());
        REQUIRE((*entry)["tampering"]["total"].get<bool>() == false);
        REQUIRE((*entry)["tampering"]["request_line_capitalization"] == true);
    }
    SECTION("For non-altered request line") {
        response->body = "{\"request_line\": \"GET / HTTP/1.1\"}";
        ooni::compare_headers_response({ {"foo", "bar"} },
                                       response,
                                       entry,
                                       Logger::global());
        REQUIRE((*entry)["tampering"]["total"].get<bool>() == false);
        REQUIRE((*entry)["tampering"]["request_line_capitalization"].get<bool>() == false);
    }
    SECTION("For extra header in response") {
        response->body = "{\"headers_dict\": {\"foo\": \"bar\", \
                                              \"foo2\": \"bar2\"}}";
        ooni::compare_headers_response({ {"foo", "bar"} },
                                       response,
                                       entry,
                                       Logger::global());
        REQUIRE((*entry)["tampering"]["header_name_diff"] == std::set<std::string>{"foo2"});
        REQUIRE((*entry)["tampering"]["header_field_name"] == true);
    }
    SECTION("For missing header in response") {
        response->body = "{\"headers_dict\": {\"foo2\": \"bar2\"}}";
        ooni::compare_headers_response({ {"foo", "bar"}, {"foo2", "bar2"} },
                                       response,
                                       entry,
                                       Logger::global());
        REQUIRE((*entry)["tampering"]["header_name_diff"] == std::set<std::string>{"foo"});
        REQUIRE((*entry)["tampering"]["header_field_name"] == true);
    }
    SECTION("For matching headers") {
        response->body = "{\"headers_dict\": {\"foo\": \"bar\"}}";
        ooni::compare_headers_response({ {"foo", "bar"} },
                                       response,
                                       entry,
                                       Logger::global());
        REQUIRE((*entry)["tampering"]["header_name_diff"] == std::set<std::string>{});
        REQUIRE((*entry)["tampering"]["header_field_name"].get<bool>() == false);
    }
}
