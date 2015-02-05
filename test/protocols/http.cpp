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

#include <ight/common/check_connectivity.hpp>
#include <ight/common/poller.hpp>
#include <ight/common/log.hpp>
#include <ight/protocols/http.hpp>

using namespace ight::common::check_connectivity;
using namespace ight::common::error;
using namespace ight::common::pointer;
using namespace ight::common::settings;
using namespace ight::net::buffer;
using namespace ight::protocols;

//
// ResponseParser unit test
//

TEST_CASE("We don't leak when we receive an invalid message") {

    //ight_set_verbose(1);

    http::ResponseParser parser;
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

    //ight_set_verbose(1);

    http::ResponseParser parser;
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

    //ight_set_verbose(1);

    auto parser = new http::ResponseParser();
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

TEST_CASE("HTTP stream works as expected") {
    if (Network::is_down()) {
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

TEST_CASE("HTTP stream is robust to EOF") {

    //ight_set_verbose(1);

    // We simulate the receipt of a message terminated by EOF followed by
    // an EOF so that stream emits in sequence "end" followed by "error(0)"
    // to check whether the code is prepared for the case in which the
    // "end" handler deletes the stream.

    auto stream = new http::Stream(Settings{
        {"dumb_transport", "1"},
    });
    stream->on_error([](Error) {
        /* nothing */
    });
    stream->on_end([stream]() {
        delete stream;
    });

    auto transport = stream->get_transport();

    stream->on_connect([stream, &transport]() {
        auto data = std::make_shared<Buffer>();

        *data << "HTTP/1.1 200 Ok\r\n";
        *data << "Content-Type: text/plain\r\n";
        *data << "Connection: close\r\n";
        *data << "Server: Antani/1.0.0.0\r\n";
        *data << "\r\n";
        *data << "1234567";

        transport->emit_data(data);
        transport->emit_error(0);
    });

    transport->emit_connect();
}

TEST_CASE("HTTP stream works as expected when using Tor") {
    if (Network::is_down()) {
        return;
    }
    ight_set_verbose(1);
    auto stream = std::make_shared<http::Stream>(Settings{
        {"address", "www.google.com"},
        {"port", "80"},
        {"socks5_proxy", "127.0.0.1:9050"},
    });
    stream->set_timeout(1.0);
    stream->on_error([&](Error e) {
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
    if (Network::is_down()) {
        return;
    }
    //ight_set_verbose(1);
    auto stream = std::make_shared<http::Stream>(Settings{
        {"address", "nexa.polito.it"},
        {"port", "81"},
    });
    stream->set_timeout(1.0);
    stream->on_error([&](Error e) {
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
    Buffer buffer;
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
    }, "", [&](Error error, http::Response&& response) {
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

TEST_CASE("HTTP request behaves correctly when EOF indicates body END") {
    //ight_set_verbose(1);

    auto called = 0;

    //
    // TODO: find a way to prevent a connection to nexa.polito.it when
    // this test run, possibly creating a stub for connect() just as
    // we created stubs for many libevent APIs.
    //

    http::Request r({
        {"url", "http://nexa.polito.it/"},
        {"method", "GET"},
        {"http_version", "HTTP/1.1"},
    }, {
        {"Accept", "*/*"},
    }, "", [&called](Error, http::Response&&) {
        ++called;
    });

    auto stream = r.get_stream();
    auto transport = stream->get_transport();

    transport->emit_connect();

    SharedPointer<Buffer> data = std::make_shared<Buffer>();
    *data << "HTTP/1.1 200 Ok\r\n";
    *data << "Content-Type: text/plain\r\n";
    *data << "Connection: close\r\n";
    *data << "Server: Antani/1.0.0.0\r\n";
    *data << "\r\n";
    *data << "1234567";
    transport->emit_data(data);
    transport->emit_error(0);

    REQUIRE(called == 1);
}

TEST_CASE("HTTP Request correctly receives errors") {
    ight_set_verbose(1);
    http::Request r({
        {"url", "http://nexa.polito.it:81/robots.txt"},
        {"method", "GET"},
        {"http_version", "HTTP/1.1"},
    }, {
        {"Accept", "*/*"},
    }, "", [&](Error error, http::Response&& response) {
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
    }, "", [&](Error error, http::Response&& response) {
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
    }, "", [&](Error, http::Response&& response) {
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
    }, "", [&](Error, http::Response&& response) {
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
    }, "", [&](Error, http::Response&& response) {
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
    }, "", [&](Error error, http::Response&& response) {
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
    }, "", [&](Error error, http::Response&& response) {
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
    }, "", [&](Error error, http::Response&& response) {
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
                [](Error error, http::Response&& response) {
        std::cout << "Error: " << error.error << std::endl;
        std::cout << response.body.read<char>() << "\r\n";
        std::cout << "[snip]\r\n";
        ight_break_loop();
    });

    ight_loop();
}

TEST_CASE("Behavior is correct when only tor_socks_port is specified") {
    //ight_set_verbose(1);

    Settings settings{
        {"method", "POST"},
        {"http_version", "HTTP/1.1"},
        {"tor_socks_port", "9055"},
    };

    settings["url"] = "httpo://nkvphnp3p6agi5qq.onion/bouncer";
    http::Request r1{settings, {
        {"Accept", "*/*"},
    }, "{\"test-helpers\": [\"dns\"]}", [](Error, http::Response&&) {
        /* nothing */
    }};

    settings["url"] = "http://ooni.torproject.org/";
    http::Request r2{settings, {
        {"Accept", "*/*"},
    }, "{\"test-helpers\": [\"dns\"]}", [](Error, http::Response&&) {
        /* nothing */
    }};

    REQUIRE(r1.socks5_address() == "127.0.0.1");
    REQUIRE(r1.socks5_port() == "9055");
    REQUIRE(r2.socks5_address() == "");
    REQUIRE(r2.socks5_port() == "");
}

TEST_CASE("Behavior is correct with both tor_socks_port and socks5_proxy") {
    //ight_set_verbose(1);

    Settings settings{
        {"method", "POST"},
        {"http_version", "HTTP/1.1"},
        {"tor_socks_port", "9999"},
        {"socks5_proxy", "127.0.0.1:9055"},
    };

    settings["url"] = "httpo://nkvphnp3p6agi5qq.onion/bouncer";
    http::Request r1{settings, {
        {"Accept", "*/*"},
    }, "{\"test-helpers\": [\"dns\"]}", [](Error, http::Response&&) {
        /* nothing */
    }};

    settings["url"] = "http://ooni.torproject.org/";
    http::Request r2{settings, {
        {"Accept", "*/*"},
    }, "{\"test-helpers\": [\"dns\"]}", [](Error, http::Response&&) {
        /* nothing */
    }};

    REQUIRE(r1.socks5_address() == "127.0.0.1");
    REQUIRE(r1.socks5_port() == "9999");
    REQUIRE(r2.socks5_address() == "127.0.0.1");
    REQUIRE(r2.socks5_port() == "9055");
}

TEST_CASE("Behavior is corrent when only socks5_proxy is specified") {
    //ight_set_verbose(1);

    Settings settings{
        {"method", "POST"},
        {"http_version", "HTTP/1.1"},
        {"socks5_proxy", "127.0.0.1:9055"},
    };

    settings["url"] = "httpo://nkvphnp3p6agi5qq.onion/bouncer";
    http::Request r1{settings, {
        {"Accept", "*/*"},
    }, "{\"test-helpers\": [\"dns\"]}", [](Error, http::Response&&) {
        /* nothing */
    }};

    settings["url"] = "http://ooni.torproject.org/";
    http::Request r2{settings, {
        {"Accept", "*/*"},
    }, "{\"test-helpers\": [\"dns\"]}", [](Error, http::Response&&) {
        /* nothing */
    }};

    REQUIRE(r1.socks5_address() == "127.0.0.1");
    REQUIRE(r1.socks5_port() == "9055");
    REQUIRE(r2.socks5_address() == "127.0.0.1");
    REQUIRE(r2.socks5_port() == "9055");
}

TEST_CASE("Behavior is OK w/o tor_socks_port and socks5_proxy") {
    //ight_set_verbose(1);

    Settings settings{
        {"method", "POST"},
        {"http_version", "HTTP/1.1"},
    };

    settings["url"] = "httpo://nkvphnp3p6agi5qq.onion/bouncer";
    http::Request r1{settings, {
        {"Accept", "*/*"},
    }, "{\"test-helpers\": [\"dns\"]}", [](Error, http::Response&&) {
        /* nothing */
    }};

    settings["url"] = "http://ooni.torproject.org/";
    http::Request r2{settings, {
        {"Accept", "*/*"},
    }, "{\"test-helpers\": [\"dns\"]}", [](Error, http::Response&&) {
        /* nothing */
    }};

    REQUIRE(r1.socks5_address() == "127.0.0.1");
    REQUIRE(r1.socks5_port() == "9050");
    REQUIRE(r2.socks5_address() == "");
    REQUIRE(r2.socks5_port() == "");
}

TEST_CASE("Make sure that settings are not modified") {
    ight_set_verbose(1);
    auto client = http::Client();

    Settings settings{
        {"url", "httpo://nkvphnp3p6agi5qq.onion/bouncer"},
        {"method", "POST"},
        {"http_version", "HTTP/1.1"},
        {"tor_socks_port", "9999"},
    };

    client.request(settings, {
        {"Accept", "*/*"},
    }, "{\"test-helpers\": [\"dns\"]}",
                [](Error error, http::Response&& response) {
        // XXX: assumes that Tor is not running on port 9999
        REQUIRE(error.error != 0);
        std::cout << "Error: " << error.error << std::endl;
        std::cout << response.body.read<char>() << "\r\n";
        std::cout << "[snip]\r\n";
        ight_break_loop();
    });

    ight_loop();

    // Make sure that no changes were made
    for (auto& iter : settings) {
        auto ok = iter.first == "url" || iter.first == "method" ||
              iter.first == "http_version" || iter.first == "tor_socks_port";
        REQUIRE(ok);
    }
}
