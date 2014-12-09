/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

//
// Regression tests for `net/http.hpp` and `net/http.cpp`.
//

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include "common/log.h"
#include "protocols/http.hpp"

//
// ResponseParser unit test
//

TEST_CASE("The HTTP response parser works as expected")
{
    auto data = std::string();
    auto parser = ight::protocols::http::ResponseParser();
    auto body = std::string();

    /*ight_set_verbose(1);*/

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
        ight_debug("%c\n", c);
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
        ight_debug("%c\n", c);
        parser.feed(c);
    }
}
