// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

//
// Regression tests for `protocols/http.hpp` and `protocols/http.cpp`.
//

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <measurement_kit/common.hpp>
#include <measurement_kit/http.hpp>

#include "src/http/stream.hpp"
#include "src/common/check_connectivity.hpp"

using namespace mk;
using namespace mk::net;
using namespace mk::http;

TEST_CASE("HTTP stream works as expected") {
    if (CheckConnectivity::is_down()) {
        return;
    }
    auto stream = std::make_shared<Stream>(Settings{
        {"address", "www.google.com"}, {"port", "80"},
    });
    stream->on_connect([&]() {
        mk::debug("Connection made... sending request");
        *stream << "GET /robots.txt HTTP/1.1\r\n"
                << "Host: www.google.com\r\n"
                << "Connection: close\r\n"
                << "\r\n";
        stream->on_flush([]() {
            mk::debug("Request sent... waiting for response");
        });
        stream->on_headers_complete(
            [&](unsigned short major, unsigned short minor, unsigned int status,
                std::string &&reason, Headers &&headers) {
                std::cout << "HTTP/" << major << "." << minor << " " << status
                          << " " << reason << "\r\n";
                for (auto &kv : headers) {
                    std::cout << kv.first << ": " << kv.second << "\r\n";
                }
                std::cout << "\r\n";
                stream->on_end([&](void) {
                    std::cout << "\r\n";
                    stream->close();
                    mk::break_loop();
                });
                stream->on_body([&](std::string && /*chunk*/) {
                    // std::cout << chunk;
                });
            });
    });
    mk::loop();
}

TEST_CASE("HTTP stream is robust to EOF") {

    // We simulate the receipt of a message terminated by EOF followed by
    // an EOF so that stream emits in sequence "end" followed by "error(0)"
    // to check whether the code is prepared for the case in which the
    // "end" handler deletes the stream.

    auto stream = new Stream(Settings{
        {"dumb_transport", "1"},
    });
    stream->on_error([](Error) {
        /* nothing */
    });
    stream->on_end([stream]() { delete stream; });

    auto transport = stream->get_transport();

    stream->on_connect([stream, &transport]() {
        Buffer data;

        data << "HTTP/1.1 200 Ok\r\n";
        data << "Content-Type: text/plain\r\n";
        data << "Connection: close\r\n";
        data << "Server: Antani/1.0.0.0\r\n";
        data << "\r\n";
        data << "1234567";

        transport.emit_data(data);
        transport.emit_error(NoError());
    });

    transport.emit_connect();
}

TEST_CASE("HTTP stream works as expected when using Tor") {
    if (CheckConnectivity::is_down()) {
        return;
    }
    auto stream = std::make_shared<Stream>(Settings{
        {"address", "www.google.com"},
        {"port", "80"},
        {"socks5_proxy", "127.0.0.1:9050"},
    });
    stream->set_timeout(1.0);
    stream->on_error([&](Error e) {
        mk::debug("Connection error: %d", (int)e);
        stream->close();
        mk::break_loop();
    });
    stream->on_connect([&]() {
        mk::debug("Connection made... sending request");
        *stream << "GET /robots.txt HTTP/1.1\r\n"
                << "Host: www.google.com\r\n"
                << "Connection: close\r\n"
                << "\r\n";
        stream->on_flush([]() {
            mk::debug("Request sent... waiting for response");
        });
        stream->on_headers_complete(
            [&](unsigned short major, unsigned short minor, unsigned int status,
                std::string &&reason, Headers &&headers) {
                std::cout << "HTTP/" << major << "." << minor << " " << status
                          << " " << reason << "\r\n";
                for (auto &kv : headers) {
                    std::cout << kv.first << ": " << kv.second << "\r\n";
                }
                std::cout << "\r\n";
                stream->on_end([&](void) {
                    std::cout << "\r\n";
                    stream->close();
                    mk::break_loop();
                });
                stream->on_body([&](std::string && /*chunk*/) {
                    // std::cout << chunk;
                });
            });
    });
    mk::loop();
}

TEST_CASE("HTTP stream receives connection errors") {
    if (CheckConnectivity::is_down()) {
        return;
    }
    auto stream = std::make_shared<Stream>(Settings{
        {"address", "nexa.polito.it"}, {"port", "81"},
    });
    stream->set_timeout(1.0);
    stream->on_error([&](Error e) {
        mk::debug("Connection error: %d", (int)e);
        stream->close();
        mk::break_loop();
    });
    mk::loop();
}
