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
using namespace ight::common::pointer;
using namespace ight::common;

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
    auto stream = std::make_shared<http::Stream>(Settings{
        {"address", "www.google.com"},
        {"port", "80"},
    });
    stream->on_connect([&]() {
        ight_debug("Connection made... sending request");
        *stream << "GET /robots.txt HTTP/1.1\r\n"
               << "Host: www.google.com\r\n"
               << "Connection: close\r\n"
               << "\r\n";
        stream->on_flush([]() {
            ight_debug("Request sent... waiting for response");
        });
        stream->on_headers_complete([&](unsigned short major,
                unsigned short minor, unsigned int status,
                std::string&& reason, http::Headers&& headers) {
            std::cout << "HTTP/" << major << "." << minor << " " <<
                        status << " " << reason << "\r\n";
            for (auto& kv : headers) {
                std::cout << kv.first << ": " << kv.second << "\r\n";
            }
            std::cout << "\r\n";
            stream->on_end([&](void) {
                std::cout << "\r\n";
                stream->close();
                ight_break_loop();
            });
            stream->on_body([&](std::string&& /*chunk*/) {
                //std::cout << chunk;
            });
        });
    });
    ight_loop();
}

TEST_CASE("HTTP stream works as expected when using Tor") {
    if (ight::Network::is_down()) {
        return;
    }
    ight_set_verbose(1);
    auto stream = std::make_shared<http::Stream>(Settings{
        {"address", "www.google.com"},
        {"port", "80"},
        {"socks5_proxy", "127.0.0.1:9050"},
    });
    stream->set_timeout(1.0);
    stream->on_error([&](IghtError e) {
        ight_debug("Connection error: %d", e.error);
        stream->close();
        ight_break_loop();
    });
    stream->on_connect([&]() {
        ight_debug("Connection made... sending request");
        *stream << "GET /robots.txt HTTP/1.1\r\n"
               << "Host: www.google.com\r\n"
               << "Connection: close\r\n"
               << "\r\n";
        stream->on_flush([]() {
            ight_debug("Request sent... waiting for response");
        });
        stream->on_headers_complete([&](unsigned short major,
                unsigned short minor, unsigned int status,
                std::string&& reason, http::Headers&& headers) {
            std::cout << "HTTP/" << major << "." << minor << " " <<
                        status << " " << reason << "\r\n";
            for (auto& kv : headers) {
                std::cout << kv.first << ": " << kv.second << "\r\n";
            }
            std::cout << "\r\n";
            stream->on_end([&](void) {
                std::cout << "\r\n";
                stream->close();
                ight_break_loop();
            });
            stream->on_body([&](std::string&& /*chunk*/) {
                //std::cout << chunk;
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
    auto stream = std::make_shared<http::Stream>(Settings{
        {"address", "nexa.polito.it"},
        {"port", "81"},
    });
    stream->set_timeout(1.0);
    stream->on_error([&](IghtError e) {
        ight_debug("Connection error: %d", e.error);
        stream->close();
        ight_break_loop();
    });
    ight_loop();
}

TEST_CASE("HTTP Request serializer works as expected") {
    auto serializer = http::RequestSerializer({
        {"follow_redirects", "yes"},
        {"url", "http://www.example.com/antani?clacsonato=yes#melandri"},
        {"ignore_body", "yes"},
        {"method", "GET"},
        {"http_version", "HTTP/1.0"},
    }, {
        {"User-Agent", "Antani/1.0.0.0"},
    }, "0123456789");
    IghtBuffer buffer;
    serializer.serialize(buffer);
    auto serialized = buffer.read<char>();
    std::string expect = "GET /antani?clacsonato=yes HTTP/1.0\r\n";
    expect += "User-Agent: Antani/1.0.0.0\r\n";
    expect += "Host: www.example.com\r\n";
    expect += "Content-Length: 10\r\n";
    expect += "\r\n";
    expect += "0123456789";
    REQUIRE(serialized == expect);
}

TEST_CASE("HTTP Request works as expected") {
    //ight_set_verbose(1);
    http::Request r({
        {"url", "http://www.google.com/robots.txt"},
        {"method", "GET"},
        {"http_version", "HTTP/1.1"},
    }, {
        {"Accept", "*/*"},
    }, "", [&](IghtError error, http::Response&& response) {
        if (error.error != 0) {
            std::cout << "Error: " << error.error << "\r\n";
            ight_break_loop();
            return;
        }
        std::cout << "HTTP/" << response.http_major << "."
                << response.http_minor << " " << response.status_code
                << " " << response.reason << "\r\n";
        for (auto& kv : response.headers) {
            std::cout << kv.first << ": " << kv.second << "\r\n";
        }
        std::cout << "\r\n";
        std::cout << response.body.read<char>(128) << "\r\n";
        std::cout << "[snip]\r\n";
        ight_break_loop();
    });
    ight_loop();
}

TEST_CASE("HTTP Request correctly receives errors") {
    ight_set_verbose(1);
    http::Request r({
        {"url", "http://nexa.polito.it:81/robots.txt"},
        {"method", "GET"},
        {"http_version", "HTTP/1.1"},
    }, {
        {"Accept", "*/*"},
    }, "", [&](IghtError error, http::Response&& response) {
        if (error.error) {
            std::cout << "Error: " << error.error << "\r\n";
            ight_break_loop();
            return;
        }
        std::cout << "HTTP/" << response.http_major << "."
                << response.http_minor << " " << response.status_code
                << " " << response.reason << "\r\n";
        for (auto& kv : response.headers) {
            std::cout << kv.first << ": " << kv.second << "\r\n";
        }
        std::cout << "\r\n";
        std::cout << response.body.read<char>(128) << "\r\n";
        std::cout << "[snip]\r\n";
        ight_break_loop();
    });
    ight_loop();
}

TEST_CASE("HTTP Request works as expected over Tor") {
    ight_set_verbose(1);
    http::Request r({
        {"url", "http://www.google.com/robots.txt"},
        {"method", "GET"},
        {"http_version", "HTTP/1.1"},
        {"socks5_proxy", "127.0.0.1:9050"},
    }, {
        {"Accept", "*/*"},
    }, "", [&](IghtError error, http::Response&& response) {
        if (error.error != 0) {
            std::cout << "Error: " << error.error << "\r\n";
            ight_break_loop();
            return;
        }
        std::cout << "HTTP/" << response.http_major << "."
                << response.http_minor << " " << response.status_code
                << " " << response.reason << "\r\n";
        for (auto& kv : response.headers) {
            std::cout << kv.first << ": " << kv.second << "\r\n";
        }
        std::cout << "\r\n";
        std::cout << response.body.read<char>(128) << "\r\n";
        std::cout << "[snip]\r\n";
        ight_break_loop();
    });
    ight_loop();
}

TEST_CASE("HTTP Client works as expected") {
    //ight_set_verbose(1);
    auto client = http::Client();
    auto count = 0;

    client.request({
        {"url", "http://www.google.com/robots.txt"},
        {"method", "GET"},
        {"http_version", "HTTP/1.1"},
        {"Connection", "close"},
    }, {
        {"Accept", "*/*"},
    }, "", [&](IghtError, http::Response&& response) {
        std::cout << "Google:\r\n";
        std::cout << response.body.read<char>(128) << "\r\n";
        std::cout << "[snip]\r\n";
        if (++count >= 3) {
            ight_break_loop();
        }
    });

    client.request({
        {"url", "http://www.neubot.org/robots.txt"},
        {"method", "GET"},
        {"http_version", "HTTP/1.1"},
    }, {
        {"Accept", "*/*"},
    }, "", [&](IghtError, http::Response&& response) {
        std::cout << response.body.read<char>(128) << "\r\n";
        std::cout << "[snip]\r\n";
        if (++count >= 3) {
            ight_break_loop();
        }
    });

    client.request({
        {"url", "http://www.torproject.org/robots.txt"},
        {"method", "GET"},
        {"http_version", "HTTP/1.1"},
    }, {
        {"Accept", "*/*"},
    }, "", [&](IghtError, http::Response&& response) {
        std::cout << response.body.read<char>(128) << "\r\n";
        std::cout << "[snip]\r\n";
        if (++count >= 3) {
            ight_break_loop();
        }
    });

    ight_loop();
}

TEST_CASE("HTTP Client works as expected over Tor") {
    ight_set_verbose(1);
    auto client = http::Client();
    auto count = 0;

    client.request({
        {"url", "http://www.google.com/robots.txt"},
        {"method", "GET"},
        {"http_version", "HTTP/1.1"},
        {"Connection", "close"},
        {"socks5_proxy", "127.0.0.1:9050"},
    }, {
        {"Accept", "*/*"},
    }, "", [&](IghtError error, http::Response&& response) {
        std::cout << "Error: " << error.error << std::endl;
        std::cout << "Google:\r\n";
        std::cout << response.body.read<char>(128) << "\r\n";
        std::cout << "[snip]\r\n";
        if (++count >= 3) {
            ight_break_loop();
        }
    });

    client.request({
        {"url", "http://www.neubot.org/robots.txt"},
        {"method", "GET"},
        {"http_version", "HTTP/1.1"},
        {"socks5_proxy", "127.0.0.1:9050"},
    }, {
        {"Accept", "*/*"},
    }, "", [&](IghtError error, http::Response&& response) {
        std::cout << "Error: " << error.error << std::endl;
        std::cout << response.body.read<char>(128) << "\r\n";
        std::cout << "[snip]\r\n";
        if (++count >= 3) {
            ight_break_loop();
        }
    });

    client.request({
        {"url", "http://www.torproject.org/robots.txt"},
        {"method", "GET"},
        {"http_version", "HTTP/1.1"},
        {"socks5_proxy", "127.0.0.1:9050"},
    }, {
        {"Accept", "*/*"},
    }, "", [&](IghtError error, http::Response&& response) {
        std::cout << "Error: " << error.error << std::endl;
        std::cout << response.body.read<char>(128) << "\r\n";
        std::cout << "[snip]\r\n";
        if (++count >= 3) {
            ight_break_loop();
        }
    });

    ight_loop();
}

TEST_CASE("Make sure that we can access OONI's bouncer using httpo://...") {
    ight_set_verbose(1);
    auto client = http::Client();

    client.request({
        {"url", "httpo://nkvphnp3p6agi5qq.onion/bouncer"},
        {"method", "POST"},
        {"http_version", "HTTP/1.1"},
    }, {
        {"Accept", "*/*"},
    }, "{\"test-helpers\": [\"dns\"]}",
                [](IghtError error, http::Response&& response) {
        std::cout << "Error: " << error.error << std::endl;
        std::cout << response.body.read<char>() << "\r\n";
        std::cout << "[snip]\r\n";
        ight_break_loop();
    });

    ight_loop();
}

TEST_CASE("Make sure that httpo://... honours tor_socks_port") {
    ight_set_verbose(1);
    auto client = http::Client();

    client.request({
        {"url", "httpo://nkvphnp3p6agi5qq.onion/bouncer"},
        {"method", "POST"},
        {"http_version", "HTTP/1.1"},
        {"tor_socks_port", "9999"},
    }, {
        {"Accept", "*/*"},
    }, "{\"test-helpers\": [\"dns\"]}",
                [](IghtError error, http::Response&& response) {
        // XXX: assumes that Tor is not running on port 9999
        REQUIRE(error.error != 0);
        std::cout << "Error: " << error.error << std::endl;
        std::cout << response.body.read<char>() << "\r\n";
        std::cout << "[snip]\r\n";
        ight_break_loop();
    });

    ight_loop();
}

TEST_CASE("Ensure that tor_socks_port takes precedence over socks5_proxy") {
    ight_set_verbose(1);
    auto client = http::Client();

    client.request({
        {"url", "httpo://nkvphnp3p6agi5qq.onion/bouncer"},
        {"method", "POST"},
        {"http_version", "HTTP/1.1"},
        {"socks5_proxy", "127.0.0.1:9050"},
        {"tor_socks_port", "9999"},
    }, {
        {"Accept", "*/*"},
    }, "{\"test-helpers\": [\"dns\"]}",
                [](IghtError error, http::Response&& response) {
        // XXX: assumes that Tor is not running on port 9999
        REQUIRE(error.error != 0);
        std::cout << "Error: " << error.error << std::endl;
        std::cout << response.body.read<char>() << "\r\n";
        std::cout << "[snip]\r\n";
        ight_break_loop();
    });

    ight_loop();
}

TEST_CASE("Ensure that socks5_proxy is honoured for httpo://...") {
    ight_set_verbose(1);
    auto client = http::Client();

    client.request({
        {"url", "httpo://nkvphnp3p6agi5qq.onion/bouncer"},
        {"method", "POST"},
        {"http_version", "HTTP/1.1"},
        {"socks5_proxy", "127.0.0.1:9999"},
    }, {
        {"Accept", "*/*"},
    }, "{\"test-helpers\": [\"dns\"]}",
                [](IghtError error, http::Response&& response) {
        // XXX: assumes that Tor is not running on port 9999
        REQUIRE(error.error != 0);
        std::cout << "Error: " << error.error << std::endl;
        std::cout << response.body.read<char>() << "\r\n";
        std::cout << "[snip]\r\n";
        ight_break_loop();
    });

    ight_loop();
}
