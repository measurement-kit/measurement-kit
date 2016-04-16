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

// TODO: Remove the empty test cases after refactoring / cleanup

TEST_CASE("HTTP stream works as expected") {
    // duplicate of t/h/request.cpp's 'http::request works as expected'
}

TEST_CASE("HTTP stream is robust to EOF") {

    // We simulate the receipt of a message terminated by EOF followed by
    // an EOF so that stream emits in sequence "end" followed by "error(0)"
    // to check whether the code is prepared for the case in which the
    // "end" handler deletes the stream.

    auto stream = new Stream(Settings{
        {"dumb_transport", 1},
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

        transport->emit_data(data);
        transport->emit_error(EofError());
    });

    transport->emit_connect();
}

TEST_CASE("HTTP stream works as expected when using Tor") {
    // duplicate of t/h/request.cpp 'http::request works as expected over Tor'
}

TEST_CASE("HTTP stream receives connection errors") {
    // duplicate of t/h/request.cpp 'http::request correctly receives errors'
}
