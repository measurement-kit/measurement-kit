// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "../src/libmeasurement_kit/ext/catch.hpp"

#include "../src/libmeasurement_kit/http/response_parser.hpp"

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

    REQUIRE(parser.feed(data) == ParserInvalidConstantError());
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

    REQUIRE(parser.feed(data) == UpgradeError());
}

TEST_CASE("ResponseParserNg works as expected with content-length") {
    std::string data;
    ResponseParserNg parser;

    data = "";
    data += "HTTP/1.2 200 Ok\r\n";
    data += "Content-Type: text/plain\r\n";
    data += "Content-Length: 7\r\n";
    data += "Server: Antani/1.0.0.0\r\n";
    data += "\r\n";
    data += "1234567";

    for (auto c : data) {
        mk::debug("%c\n", c);
        Error err = parser.feed(c);
        REQUIRE((
            err == ParsingHeadersInProgressError() or
            err == ParsingBodyInProgressError() or
            err == NoError() or
            err == PausedAfterParsingHeadersError()
        ));
        if (err == PausedAfterParsingHeadersError()) {
            err = parser.parse();
            REQUIRE(err == ParsingBodyInProgressError());
        }
    }

    REQUIRE(parser.response.http_major == 1);
    REQUIRE(parser.response.http_minor == 2);
    REQUIRE(parser.response.status_code == 200);
    REQUIRE(parser.response.reason == "Ok");
    REQUIRE(parser.response.headers.size() == 3);
    REQUIRE(parser.response.headers.at("Content-Type") == "text/plain");
    REQUIRE(parser.response.headers.at("Content-Length") == "7");
    REQUIRE(parser.response.headers.at("Server") == "Antani/1.0.0.0");
    REQUIRE(parser.response.body == "1234567");

    /*
     * When the parser has reached the final state it is paused and
     * we cannot convince it to restart parsing but we need to create
     * a new parser object instead.
     */
    REQUIRE(parser.feed("XXX XXX XXX\r\n\r\n") == ParserPausedError());
}

TEST_CASE("ResponseParserNg works as expected with chunked body") {
    std::string data;
    ResponseParserNg parser;

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

    for (auto c : data) {
        mk::debug("%c\n", c);
        Error err = parser.feed(c);
        REQUIRE((
            err == ParsingHeadersInProgressError() or
            err == ParsingBodyInProgressError() or
            err == NoError() or
            err == PausedAfterParsingHeadersError()
        ));
        if (err == PausedAfterParsingHeadersError()) {
            err = parser.parse();
            REQUIRE(err == ParsingBodyInProgressError());
        }
    }

    REQUIRE(parser.response.http_major == 1);
    REQUIRE(parser.response.http_minor == 1);
    REQUIRE(parser.response.status_code == 202);
    REQUIRE(parser.response.reason == "Accepted");
    REQUIRE(parser.response.headers.size() == 3);
    REQUIRE(parser.response.headers.at("Content-Type") == "text/html");
    REQUIRE(parser.response.headers.at("Transfer-Encoding") == "chunked");
    REQUIRE(parser.response.headers.at("Server") == "Antani/2.0.0.0");
    REQUIRE(parser.response.body == "abcabcabc");
}

TEST_CASE("ResponseParserNg eof() works as expected") {
    ResponseParserNg parser;
    std::string data;

    data = "";
    data += "HTTP/1.1 200 Ok\r\n";
    data += "Content-Type: text/plain\r\n";
    data += "Connection: close\r\n";
    data += "Server: Antani/1.0.0.0\r\n";
    data += "\r\n";
    data += "1234567";

    Error err = parser.feed(data);
    REQUIRE(err == PausedAfterParsingHeadersError());
    err = parser.parse();
    REQUIRE(err == ParsingBodyInProgressError());
    REQUIRE(parser.eof() == NoError());
}
