// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN

#include "src/net/socks5.hpp"
#include "src/ext/Catch/single_include/catch.hpp"
#include <measurement_kit/net.hpp>
using namespace mk;
using namespace mk::net;

TEST_CASE("format_auth_request() works as expected") {
    Buffer buffer = socks5_format_auth_request();
    REQUIRE(buffer.length() == 3);
    std::string message = buffer.read();
    REQUIRE(message[0] == '\5');
    REQUIRE(message[1] == '\1');
    REQUIRE(message[2] == '\0');
}

TEST_CASE("parse_auth_response() works as expected") {

    SECTION("When there is no input at all") {
        Buffer input;
        ErrorOr<bool> rc = socks5_parse_auth_response(input);
        REQUIRE(rc.as_error() == NoError());
        REQUIRE(rc.as_value() == false);
    }

    SECTION("When there is just one byte of data") {
        Buffer input;
        input.write_uint8(5);
        ErrorOr<bool> rc = socks5_parse_auth_response(input);
        REQUIRE(rc.as_error() == NoError());
        REQUIRE(rc.as_value() == false);
    }

    SECTION("When the version is wrong") {
        Buffer input;
        input.write_uint8(4);
        input.write_uint8(0);
        ErrorOr<bool> rc = socks5_parse_auth_response(input);
        REQUIRE(rc.as_error() == BadSocksVersionError());
        REQUIRE_THROWS_AS(rc.as_value(), Error);
    }

    SECTION("When the preferred_auth is wrong") {
        Buffer input;
        input.write_uint8(5);
        input.write_uint8(16);
        ErrorOr<bool> rc = socks5_parse_auth_response(input);
        REQUIRE(rc.as_error() == NoAvailableSocksAuthenticationError());
        REQUIRE_THROWS_AS(rc.as_value(), Error);
    }

    SECTION("When the input is OK") {
        Buffer input;
        input.write_uint8(5);
        input.write_uint8(0);
        ErrorOr<bool> rc = socks5_parse_auth_response(input);
        REQUIRE(rc.as_error() == NoError());
        REQUIRE(rc.as_value() == true);
    }
}

TEST_CASE("format_connect_request() works as expected") {
    SECTION("When the address is too long") {
        ErrorOr<Buffer> rc = socks5_format_connect_request({
            {"net/address", std::string(1024, 'A')},
        });
        REQUIRE(rc.as_error() == SocksAddressTooLongError());
        REQUIRE_THROWS_AS(rc.as_value(), Error);
    }

    SECTION("When the port number is negative") {
        ErrorOr<Buffer> rc = socks5_format_connect_request({
            {"net/address", "130.192.91.211"}, {"net/port", -1},
        });
        REQUIRE(rc.as_error() == SocksInvalidPortError());
        REQUIRE_THROWS_AS(rc.as_value(), Error);
    }

    SECTION("When the port number is too large") {
        ErrorOr<Buffer> rc = socks5_format_connect_request({
            {"net/address", "130.192.91.211"}, {"net/port", 65536},
        });
        REQUIRE(rc.as_error() == SocksInvalidPortError());
        REQUIRE_THROWS_AS(rc.as_value(), Error);
    }

    SECTION("When input is OK") {
        std::string address = "130.192.91.211";
        uint16_t orig_port = 8080;
        ErrorOr<Buffer> rc = socks5_format_connect_request({
            {"net/address", address}, {"net/port", orig_port},
        });
        REQUIRE(rc.as_error() == NoError());
        std::string msg = rc->read(5 + address.length());
        REQUIRE(msg[0] == '\5');
        REQUIRE(msg[1] == '\1');
        REQUIRE(msg[2] == '\0');
        REQUIRE(msg[3] == '\3');
        REQUIRE(msg[4] == address.length());
        REQUIRE(msg.substr(5, address.length()) == address);
        // XXX This part of the test is currently not possible:
        // uint16_t port = rc->read_uint16();
        // REQUIRE(port == orig_port);
    }
}

TEST_CASE("parse_connect_response() works as expected") {

    SECTION("When there are less than five bytes of input") {
        Buffer input("ABCD");
        ErrorOr<bool> rc = socks5_parse_connect_response(input);
        REQUIRE(rc.as_error() == NoError());
        REQUIRE(rc.as_value() == false);
    }

    SECTION("When the version is not OK") {
        Buffer input;
        input.write_uint8(4);
        input.write_uint8(0);
        input.write_uint8(0);
        input.write_uint8(3);
        input.write_uint8(0);
        ErrorOr<bool> rc = socks5_parse_connect_response(input);
        REQUIRE(rc.as_error() == BadSocksVersionError());
        REQUIRE_THROWS_AS(rc.as_value(), Error);
    }

    SECTION("When there was a network error") {
        Buffer input;
        input.write_uint8(5);
        input.write_uint8(1);
        input.write_uint8(0);
        input.write_uint8(3);
        input.write_uint8(0);
        ErrorOr<bool> rc = socks5_parse_connect_response(input);
        REQUIRE(rc.as_error() == SocksError());
        REQUIRE_THROWS_AS(rc.as_value(), Error);
    }

    SECTION("When the reserved field is invalid") {
        Buffer input;
        input.write_uint8(5);
        input.write_uint8(0);
        input.write_uint8(1);
        input.write_uint8(3);
        input.write_uint8(0);
        ErrorOr<bool> rc = socks5_parse_connect_response(input);
        REQUIRE(rc.as_error() == BadSocksReservedFieldError());
        REQUIRE_THROWS_AS(rc.as_value(), Error);
    }

    SECTION("When the atype field is invalid") {
        Buffer input;
        input.write_uint8(5);
        input.write_uint8(0);
        input.write_uint8(0);
        input.write_uint8(44);
        input.write_uint8(0);
        ErrorOr<bool> rc = socks5_parse_connect_response(input);
        REQUIRE(rc.as_error() == BadSocksAtypeValueError());
        REQUIRE_THROWS_AS(rc.as_value(), Error);
    }

    SECTION("When not the whole message was read") {
        Buffer input;
        input.write_uint8(5);
        input.write_uint8(0);
        input.write_uint8(0);
        input.write_uint8(3);
        input.write_uint8(6);
        ErrorOr<bool> rc = socks5_parse_connect_response(input);
        REQUIRE(rc.as_error() == NoError());
        REQUIRE(rc.as_value() == false);
    }

    SECTION("When the message contains an IPv4 address") {
        Buffer input;
        input.write_uint8(5);
        input.write_uint8(0);
        input.write_uint8(0);
        input.write_uint8(1);
        // <IPv4>
        input.write_uint8(0);
        input.write_uint8(0);
        input.write_uint8(0);
        input.write_uint8(0);
        // </IPv4>
        input.write_uint16(8080);
        ErrorOr<bool> rc = socks5_parse_connect_response(input);
        REQUIRE(rc.as_error() == NoError());
        REQUIRE(rc.as_value() == true);
        REQUIRE(input.length() == 0);
    }

    SECTION("When the message contains a string address") {
        Buffer input;
        input.write_uint8(5);
        input.write_uint8(0);
        input.write_uint8(0);
        input.write_uint8(3);
        // <len+string>
        input.write_uint8(5);
        input.write_uint8('x');
        input.write_uint8('.');
        input.write_uint8('o');
        input.write_uint8('r');
        input.write_uint8('g');
        // </len+string>
        input.write_uint16(8080);
        ErrorOr<bool> rc = socks5_parse_connect_response(input);
        REQUIRE(rc.as_error() == NoError());
        REQUIRE(rc.as_value() == true);
        REQUIRE(input.length() == 0);
    }

    SECTION("When the message contains a IPv6 address") {
        Buffer input;
        input.write_uint8(5);
        input.write_uint8(0);
        input.write_uint8(0);
        input.write_uint8(4);
        // <IPv6>
        input.write_uint8(0), input.write_uint8(0), input.write_uint8(0),
            input.write_uint8(0);
        input.write_uint8(0), input.write_uint8(0), input.write_uint8(0),
            input.write_uint8(0);
        input.write_uint8(0), input.write_uint8(0), input.write_uint8(0),
            input.write_uint8(0);
        input.write_uint8(0), input.write_uint8(0), input.write_uint8(0),
            input.write_uint8(0);
        // </IPv6>
        input.write_uint16(8080);
        ErrorOr<bool> rc = socks5_parse_connect_response(input);
        REQUIRE(rc.as_error() == NoError());
        REQUIRE(rc.as_value() == true);
        REQUIRE(input.length() == 0);
    }
}
