// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

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
    // duplicate of t/h/request.cpp's 'http:request_sendrecv() behaves
    // correctly when EOF indicates body END'
}

TEST_CASE("HTTP stream works as expected when using Tor") {
    // duplicate of t/h/request.cpp 'http::request works as expected over Tor'
}

TEST_CASE("HTTP stream receives connection errors") {
    // duplicate of t/h/request.cpp 'http::request correctly receives errors'
}
