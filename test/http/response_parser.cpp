// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include "src/http/response_parser.hpp"
#include <measurement_kit/http.hpp>

using namespace mk;
using namespace mk::net;
using namespace mk::http;

TEST_CASE("ResponseParserNg deals with an invalid message") {
    ResponseParserNg parser;
    std::string data;

    data = "";
    data += "XXX XXX XXX XXX\r\n";
    data += "Content-Type: text/plain\r\n";
    data += "Connection: close\r\n";
    data += "Server: Antani/1.0.0.0\r\n";
    data += "\r\n";
    data += "1234567";

    REQUIRE_THROWS_AS(parser.feed(data), ParserError);
}

TEST_CASE("ResponseParserNg deals with an UPGRADE request") {
    ResponseParserNg parser;
    std::string data;

    data = "";
    data += "HTTP/1.1 200 Ok\r\n";
    data += "Content-Type: text/plain\r\n";
    data += "Connection: upgrade\r\n";
    data += "Upgrade: websockets\r\n";
    data += "Server: Antani/1.0.0.0\r\n";
    data += "\r\n";

    REQUIRE_THROWS_AS(parser.feed(data), UpgradeError);
}

TEST_CASE("ResponseParserNg works as expected") {
    std::string data;
    ResponseParserNg parser;
    std::string body;

    //
    // Request #1
    //

    data = "";
    data += "HTTP/1.2 200 Ok\r\n";
    data += "Content-Type: text/plain\r\n";
    data += "Content-Length: 7\r\n";
    data += "Server: Antani/1.0.0.0\r\n";
    data += "\r\n";
    data += "1234567";

    parser.on_response([](Response r) {
        REQUIRE(r.http_major == 1);
        REQUIRE(r.http_minor == 2);
        REQUIRE(r.status_code == 200);
        REQUIRE(r.reason == "Ok");
        REQUIRE(r.headers.size() == 3);
        REQUIRE(r.headers.at("Content-Type") == "text/plain");
        REQUIRE(r.headers.at("Content-Length") == "7");
        REQUIRE(r.headers.at("Server") == "Antani/1.0.0.0");
    });

    body = "";
    parser.on_body([&body](std::string s) { body += s; });
    parser.on_end([&body]() { REQUIRE(body == "1234567"); });

    for (auto c : data) {
        mk::debug("%c\n", c);
        parser.feed(c);
    }

    //
    // Request #2
    //

    data = "";
    data += "HTTP/1.1 202 Accepted\r\n";
    data += "Content-Type: text/html\r\n";
    data += "Transfer-Encoding: chunked\r\n";
    data += "Server: Antani/2.0.0.0\r\n";
    data += "\r\n";
    data += "3\r\nabc\r\n";
    data += "3\r\nabc\r\n";
    data += "3\r\nabc\r\n";
    data += "0\r\nX-Trailer: trailer\r\n\r\n";

    parser.on_response([](Response r) {
        REQUIRE(r.http_major == 1);
        REQUIRE(r.http_minor == 1);
        REQUIRE(r.status_code == 202);
        REQUIRE(r.reason == "Accepted");
        REQUIRE(r.headers.size() == 3);
        REQUIRE(r.headers.at("Content-Type") == "text/html");
        REQUIRE(r.headers.at("Transfer-Encoding") == "chunked");
        REQUIRE(r.headers.at("Server") == "Antani/2.0.0.0");
    });

    body = "";
    parser.on_body([&body](std::string s) { body += s; });
    parser.on_end([&body]() { REQUIRE(body == "abcabcabc"); });

    for (auto c : data) {
        mk::debug("%c\n", c);
        parser.feed(c);
    }
}

TEST_CASE("ResponseParserNg eof() works as expected") {
    bool called = false;
    ResponseParserNg parser;
    std::string data;

    data = "";
    data += "HTTP/1.1 200 Ok\r\n";
    data += "Content-Type: text/plain\r\n";
    data += "Connection: close\r\n";
    data += "Server: Antani/1.0.0.0\r\n";
    data += "\r\n";
    data += "1234567";

    parser.feed(data);
    parser.on_end([&called]() { called = true; });
    parser.eof();

    REQUIRE(called);
}
