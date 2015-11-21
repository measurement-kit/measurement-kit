// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

//
// Tests for src/common/utils.hpp's measurement_kit::socket_valid()
//

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <measurement_kit/common.hpp>

/*
 * Test Unix:
 */

static inline void test_valid_sockets_unix_(void) {
    REQUIRE(measurement_kit::socket_valid_unix_(0));
    REQUIRE(measurement_kit::socket_valid_unix_(1));
    REQUIRE(measurement_kit::socket_valid_unix_(2));
    /* ... */
    REQUIRE(measurement_kit::socket_valid_unix_(INT_MAX));
}

static inline void test_invalid_sockets_unix_(void) {
    REQUIRE(!measurement_kit::socket_valid_unix_(-1));
    REQUIRE(!measurement_kit::socket_valid_unix_(-2));
    /* ... */
    REQUIRE(!measurement_kit::socket_valid_unix_(INT_MIN));
}

static inline void test_normalize_sockets_unix_(void) {
    REQUIRE(measurement_kit::socket_normalize_if_invalid_unix_(-1) == -1);
    REQUIRE(measurement_kit::socket_normalize_if_invalid_unix_(-2) == -1);
    /* ... */
    REQUIRE(measurement_kit::socket_normalize_if_invalid_unix_(INT_MIN) == -1);
}

TEST_CASE("Nonnegative integers are valid sockets on Unix") {
    test_valid_sockets_unix_();
}

TEST_CASE("Negative integers are invalid sockets on Unix") {
    test_invalid_sockets_unix_();
}

TEST_CASE("Negative integers are normalized as -1 on Unix") {
    test_normalize_sockets_unix_();
}

/*
 * Test Win32 (using signed sockets as input, as used by libevent; see also
 * the long comment on this topic in `common/utils.hpp`):
 */

static inline void test_valid_sockets_win32_(void) {
    REQUIRE(measurement_kit::socket_valid_win32_(-2));
    REQUIRE(measurement_kit::socket_valid_win32_(-3));
    REQUIRE(measurement_kit::socket_valid_win32_(-4));
    /* ... */
    REQUIRE(measurement_kit::socket_valid_win32_(INTPTR_MIN + 2));
    REQUIRE(measurement_kit::socket_valid_win32_(INTPTR_MIN + 1));
    REQUIRE(measurement_kit::socket_valid_win32_(INTPTR_MIN));
    /* ... */
    REQUIRE(measurement_kit::socket_valid_win32_(0));
    REQUIRE(measurement_kit::socket_valid_win32_(1));
    REQUIRE(measurement_kit::socket_valid_win32_(2));
    /* ... */
    REQUIRE(measurement_kit::socket_valid_win32_(INTPTR_MAX - 2));
    REQUIRE(measurement_kit::socket_valid_win32_(INTPTR_MAX - 1));
    REQUIRE(measurement_kit::socket_valid_win32_(INTPTR_MAX));
}

static inline void test_invalid_sockets_win32_(void) {
    REQUIRE(!measurement_kit::socket_valid_win32_(-1));
}

static inline void test_normalize_sockets_win32_(void) {
    REQUIRE(measurement_kit::socket_normalize_if_invalid_win32_(-1) == -1);
}

TEST_CASE("All integers but INVALID_SOCKET are valid sockets on Win32") {
    test_valid_sockets_win32_();
}

TEST_CASE("INVALID_SOCKET is an invalid socket on Win32") {
    test_invalid_sockets_win32_();
}

TEST_CASE("INVALID_SOCKET is normalized as INVALID_SOCKET on Win32") {
    test_normalize_sockets_win32_();
}

/*
 * Test the current platform:
 */

TEST_CASE("Valid sockets are correctly recognized") {
#ifdef WIN32
    test_valid_sockets_win32_();
#else
    test_valid_sockets_unix_();
#endif
}

TEST_CASE("Invalid sockets are correctly recognized") {
#ifdef WIN32
    test_invalid_sockets_win32_();
#else
    test_invalid_sockets_unix_();
#endif
}

TEST_CASE("Invalid sockets are correctly normalized to -1") {
#ifdef WIN32
    test_normalize_sockets_win32_();
#else
    test_normalize_sockets_unix_();
#endif
}
