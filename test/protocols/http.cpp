/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

//
// Regression tests for `protocols/http.hpp` and `protocols/http.cpp`.
//

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include "common/check_connectivity.hpp"
#include "common/poller.h"
#include "common/log.h"
#include "protocols/http.hpp"

using namespace ight::protocols;

//
// ResponseParser unit test
//

TEST_CASE("The HTTP response parser works as expected") {
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

TEST_CASE("HTTP stream works as expected") {
    if (ight::Network::is_down()) {
        return;
    }
    //ight_set_verbose(1);
    auto stream = http::Stream::connect("www.google.com", "80");
    stream.on_connect([&]() {
        ight_debug("Connection made... sending request");
        stream << "GET /robots.txt HTTP/1.1\r\n"
               << "Host: www.google.com\r\n"
               //<< "Connection: close\r\n"  // Cannot do this yet
               << "\r\n";
        stream.on_flush([]() {
            ight_debug("Request sent... waiting for response");
        });
        stream.on_headers_complete([&](unsigned short major,
                unsigned short minor, unsigned int status,
                std::string&& reason, http::Headers&& headers) {
            std::cout << "HTTP/" << major << "." << minor << " " <<
                        status << " " << reason << "\r\n";
            for (auto& kv : headers) {
                std::cout << kv.first << ": " << kv.second << "\r\n";
            }
            std::cout << "\r\n";
            stream.on_end([&](void) {
                std::cout << "\r\n";
                stream.close();
                ight_break_loop();
            });
            stream.on_body([&](std::string&& chunk) {
                std::cout << chunk;
            });
        });
    });
    ight_loop();
}

TEST_CASE("HTTP stream receives connection errors") {
    if (ight::Network::is_down()) {
        return;
    }
    //ight_set_verbose(1);
    auto stream = http::Stream::connect("nexa.polito.it", "81");
    stream.set_timeout(1.0);
    stream.on_error([&](IghtError e) {
        ight_debug("Connection error: %d", e.error);
        stream.close();
        ight_break_loop();
    });
    ight_loop();
}
