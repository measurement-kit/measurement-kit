// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <measurement_kit/common.hpp>
#include <measurement_kit/http.hpp>

#include "src/http/response_parser.hpp"

using namespace measurement_kit::common;
using namespace measurement_kit::net;
using namespace measurement_kit::http;

TEST_CASE("We don't leak when we receive an invalid message") {

    //measurement_kit::set_verbose(1);

    ResponseParser parser;
    std::string data;

    //
    // We pass the parser an invalid message that should trigger an
    // exception. After the exception, the internal parser should be
    // in a state by which, when the external parser is deleted,
    // the internal parser is also deleted.
    //

    data = "";
    data += "XXX XXX XXX XXX\r\n";
    data += "Content-Type: text/plain\r\n";
    data += "Connection: close\r\n";
    data += "Server: Antani/1.0.0.0\r\n";
    data += "\r\n";
    data += "1234567";

    REQUIRE_THROWS(parser.feed(data));
}

TEST_CASE("We don't leak when we receive a UPGRADE") {

    //measurement_kit::set_verbose(1);

    ResponseParser parser;
    std::string data;

    //
    // We pass the parser an invalid message that should trigger an
    // exception. After the exception, the internal parser should be
    // in a state by which, when the external parser is deleted,
    // the internal parser is also deleted.
    //

    data = "";
    data += "HTTP/1.1 200 Ok\r\n";
    data += "Content-Type: text/plain\r\n";
    data += "Connection: upgrade\r\n";
    data += "Upgrade: websockets\r\n";
    data += "Server: Antani/1.0.0.0\r\n";
    data += "\r\n";

    REQUIRE_THROWS(parser.feed(data));
}

TEST_CASE("The HTTP response parser works as expected") {
    auto data = std::string();
    auto parser = ResponseParser();
    auto body = std::string();

    /*measurement_kit::set_verbose(1);*/

    //
    // Request #1
    //

    data = "";
    data += "HTTP/1.1 200 Ok\r\n";
    data += "Content-Type: text/plain\r\n";
    data += "Content-Length: 7\r\n";
    data += "Server: Antani/1.0.0.0\r\n";
    data += "\r\n";
    data += "1234567";

    parser.on_headers_complete([](unsigned short major, unsigned short minor,
            unsigned int code, std::string&& reason, std::map<std::string,
            std::string>&& headers) {
        REQUIRE(major == 1);
        REQUIRE(minor == 1);
        REQUIRE(code == 200);
        REQUIRE(reason == "Ok");
        REQUIRE(headers.size() == 3);
        REQUIRE(headers.at("Content-Type") == "text/plain");
        REQUIRE(headers.at("Content-Length") == "7");
        REQUIRE(headers.at("Server") == "Antani/1.0.0.0");
    });

    body = "";
    parser.on_body([&](std::string s) {
        body += s;
    });

    parser.on_end([&](void) {
        REQUIRE(body == "1234567");
    });

    for (auto c : data) {
        measurement_kit::debug("%c\n", c);
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

    parser.on_headers_complete([](unsigned short major, unsigned short minor,
            unsigned int code, std::string&& reason, std::map<std::string,
            std::string>&& headers) {
        REQUIRE(major == 1);
        REQUIRE(minor == 1);
        REQUIRE(code == 202);
        REQUIRE(reason == "Accepted");
        REQUIRE(headers.size() == 3);
        REQUIRE(headers.at("Content-Type") == "text/html");
        REQUIRE(headers.at("Transfer-Encoding") == "chunked");
        REQUIRE(headers.at("Server") == "Antani/2.0.0.0");
    });

    body = "";
    parser.on_body([&](std::string s) {
        body += s;
    });

    parser.on_end([&](void) {
        REQUIRE(body == "abcabcabc");
    });

    for (auto c : data) {
        measurement_kit::debug("%c\n", c);
        parser.feed(c);
    }
}

TEST_CASE("Response parser eof() does not trigger immediate distruction") {

    //
    // Provide input data by which the end of body is signalled by
    // the connection being closed, such that invoking the parser's
    // eof() method is goind to trigger the on_end() callback.
    //
    // Then, in the callback delete the parser object, which should
    // not trigger a crash because the real parser should understand
    // that it is parsing and should delay its destruction.
    //

    //measurement_kit::set_verbose(1);

    auto parser = new ResponseParser();
    std::string data;

    data = "";
    data += "HTTP/1.1 200 Ok\r\n";
    data += "Content-Type: text/plain\r\n";
    data += "Connection: close\r\n";
    data += "Server: Antani/1.0.0.0\r\n";
    data += "\r\n";
    data += "1234567";

    parser->feed(data);

    parser->on_end([parser]() {
        delete parser;
    });
    parser->eof();
}
