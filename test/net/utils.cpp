// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include <cerrno>

#define CATCH_CONFIG_MAIN
#include "src/libmeasurement_kit/ext/catch.hpp"

#include "src/libmeasurement_kit/net/error.hpp"
#include "src/libmeasurement_kit/net/utils.hpp"

TEST_CASE("is_ipv4_addr works") {
    SECTION("on ipv4") {
        REQUIRE(mk::net::is_ipv4_addr("127.0.0.1") == true);
    }
    SECTION("on ipv6") {
        REQUIRE(mk::net::is_ipv4_addr("::42") == false);
    }
    SECTION("on hostnames") {
        REQUIRE(mk::net::is_ipv4_addr("example.com") == false);
    }
}

TEST_CASE("is_ipv6_addr works") {
    SECTION("on ipv4") {
        REQUIRE(mk::net::is_ipv6_addr("127.0.0.1") == false);
    }
    SECTION("on ipv6") {
        REQUIRE(mk::net::is_ipv6_addr("::42") == true);
    }
    SECTION("on hostnames") {
        REQUIRE(mk::net::is_ipv6_addr("example.com") == false);
    }
}

TEST_CASE("is_ip_addr works") {
    SECTION("on ipv4") {
        REQUIRE(mk::net::is_ip_addr("127.0.0.1") == true);
    }
    SECTION("on ipv6") {
        REQUIRE(mk::net::is_ip_addr("::42") == true);
    }
    SECTION("on hostnames") {
        REQUIRE(mk::net::is_ip_addr("example.com") == false);
    }
}

TEST_CASE("parse_endpoint works for IPv4") {
    SECTION("With explicit address and port") {
        auto epnt = mk::net::parse_endpoint("130.192.91.211:80", 53);
        REQUIRE(epnt->hostname == "130.192.91.211");
        REQUIRE(epnt->port == 80);
    }
    SECTION("Without the port") {
        auto epnt = mk::net::parse_endpoint("130.192.91.211", 53);
        REQUIRE(epnt->hostname == "130.192.91.211");
        REQUIRE(epnt->port == 53);
    }
    SECTION("If the hostname is not specified") {
        auto epnt = mk::net::parse_endpoint(":80", 53);
        REQUIRE(!epnt);
        REQUIRE(epnt.as_error() == mk::ValueError());
    }
}

TEST_CASE("parse_endpoint works for IPv6") {

    SECTION("With explicit address and port") {
        auto epnt = mk::net::parse_endpoint("[::1]:80", 53);
        REQUIRE(epnt->hostname == "::1");
        REQUIRE(epnt->port == 80);
    }
    SECTION("Without the port and without parentheses") {
        auto epnt = mk::net::parse_endpoint("::1", 53);
        REQUIRE(epnt->hostname == "::1");
        REQUIRE(epnt->port == 53);
    }
    SECTION("Without the port and with parentheses") {
        auto epnt = mk::net::parse_endpoint("[::1]", 53);
        REQUIRE(epnt->hostname == "::1");
        REQUIRE(epnt->port == 53);
    }

    SECTION("With explicit address and port and scope") {
        auto epnt = mk::net::parse_endpoint(R"([::1%en0]:80)", 53);
        REQUIRE(epnt->hostname == R"(::1%en0)");
        REQUIRE(epnt->port == 80);
    }
    SECTION("With scope, without the port and without parentheses") {
        /*
         * Test disabled for other platforms. It seems I cannot produce
         * a link-scope address for Linux that works here (why!?).
         *
         * The specific problem is that, for the test to work, we need
         * to convince the system inet_pton() that the input address is
         * a good link-scope address, otherwise this test fails.
         *
         *      -Simone (2017/01/15)
         */
#ifdef __APPLE__
        std::string s = R"(fe80::1%lo0)";
        auto epnt = mk::net::parse_endpoint(s, 53);
        REQUIRE(epnt->hostname == s);
        REQUIRE(epnt->port == 53);
#endif
    }
    SECTION("With scope, without the port and with parentheses") {
        auto epnt = mk::net::parse_endpoint(R"([::1%en0])", 53);
        REQUIRE(epnt->hostname == R"(::1%en0)");
        REQUIRE(epnt->port == 53);
    }

    SECTION("If the hostname is not specified") {
        auto epnt = mk::net::parse_endpoint(":80", 53);
        REQUIRE(!epnt);
        REQUIRE(epnt.as_error() == mk::ValueError());
    }
    SECTION("If the hostname is not specified with parentheses") {
        auto epnt = mk::net::parse_endpoint("[]:80", 53);
        REQUIRE(!epnt);
        REQUIRE(epnt.as_error() == mk::ValueError());
    }
}

TEST_CASE("parse_endpoint works for domain") {
    SECTION("With explicit address and port") {
        auto epnt = mk::net::parse_endpoint("www.google.com:80", 53);
        REQUIRE(epnt->hostname == "www.google.com");
        REQUIRE(epnt->port == 80);
    }
    SECTION("Without the port") {
        auto epnt = mk::net::parse_endpoint("www.google.com", 53);
        REQUIRE(epnt->hostname == "www.google.com");
        REQUIRE(epnt->port == 53);
    }
    SECTION("If the hostname is not specified") {
        auto epnt = mk::net::parse_endpoint(":80", 53);
        REQUIRE(!epnt);
        REQUIRE(epnt.as_error() == mk::ValueError());
    }
}

TEST_CASE("Serialize endpoint works correctly") {
    SECTION("For IPv4") {
        mk::net::Endpoint epnt;
        epnt.hostname = "130.192.91.211";
        epnt.port = 80;
        std::string s = mk::net::serialize_endpoint(epnt);
        REQUIRE(s == "130.192.91.211:80");
        auto maybe_epnt = mk::net::parse_endpoint(s, 22);
        REQUIRE(maybe_epnt->hostname == epnt.hostname);
        REQUIRE(maybe_epnt->port == epnt.port);
    }
    SECTION("For IPv6") {
        mk::net::Endpoint epnt;
        epnt.hostname = "::1";
        epnt.port = 80;
        std::string s = mk::net::serialize_endpoint(epnt);
        REQUIRE(s == "[::1]:80");
        auto maybe_epnt = mk::net::parse_endpoint(s, 22);
        REQUIRE(maybe_epnt->hostname == epnt.hostname);
        REQUIRE(maybe_epnt->port == epnt.port);
    }
    SECTION("For domain") {
        mk::net::Endpoint epnt;
        epnt.hostname = "www.google.com";
        epnt.port = 80;
        std::string s = mk::net::serialize_endpoint(epnt);
        REQUIRE(s == "www.google.com:80");
        auto maybe_epnt = mk::net::parse_endpoint(s, 22);
        REQUIRE(maybe_epnt->hostname == epnt.hostname);
        REQUIRE(maybe_epnt->port == epnt.port);
    }
}

TEST_CASE("IPv4 addresses are correctly reversed") {
    REQUIRE(mk::net::unreverse_ipv4("211.91.192.130.in-addr.arpa") ==
            "130.192.91.211");
    REQUIRE(mk::net::unreverse_ipv4("4.3.2.1.in-addr.arpa.") ==
            "1.2.3.4");
    REQUIRE(mk::net::unreverse_ipv4("22.177.3.149.in-addr.arpa") ==
            "149.3.177.22");
}

TEST_CASE("IPv4 addresses cannot contain numbers > 255") {
    REQUIRE(mk::net::unreverse_ipv4("254.777.254.254.in-addr.arpa") ==
            "");
    REQUIRE(mk::net::unreverse_ipv4("255.255.255.255.in-addr.arpa") ==
            "255.255.255.255");
}

TEST_CASE("IPv6 addresses are correctly reversed") {
    REQUIRE(mk::net::unreverse_ipv6(
                "b.a.9.8.7.6.5.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.8.b.d.0.1.0.0."
                "2.ip6.arpa") == "2001:0db8:0000:0000:0000:0000:0567:89ab");
    REQUIRE(mk::net::unreverse_ipv6(
                "1.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.8.b.d.0.1.0.0."
                "2.ip6.arpa.") == "2001:0db8:0000:0000:0000:0000:0000:0001");
    REQUIRE(mk::net::unreverse_ipv6(
                "4.0.0.2.0.0.0.0.0.0.0.0.0.0.0.0.8.0.8.0.2.0.0.4.0.5.4.1.0.0.a."
                "2.ip6.arpa") == "2a00:1450:4002:0808:0000:0000:0000:2004");
}

TEST_CASE("Verify that invalid input is rejected") {

    SECTION("For IPv4") {
        REQUIRE(mk::net::unreverse_ipv4("") == "");
        // First non number non dot character we break and search in-addr.arpa
        REQUIRE(mk::net::unreverse_ipv4("foobar") == "");
        // We deal correctly with good address with missing suffix
        REQUIRE(mk::net::unreverse_ipv4("4.3.2.1") == "");
        REQUIRE(mk::net::unreverse_ipv4("4.3.2.1.") == "");
    }

    SECTION("For IPv6") {
        REQUIRE(mk::net::unreverse_ipv6("") == "");
        // We encounter a character that is not a dot in position N + 1
        REQUIRE(mk::net::unreverse_ipv6("e.2.1;d.e.a.d") == "");
        // We encounter a character that is not hex in position N
        REQUIRE(mk::net::unreverse_ipv6("d.e.a.d.r.e.e.f") == "");
        // We deal correctly with good address with missing suffix
        REQUIRE(
            mk::net::unreverse_ipv6(
                "b.a.9.8.7.6.5.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.8.b.d.0.1.0.0."
                "2") == "");
        REQUIRE(
            mk::net::unreverse_ipv6(
                "b.a.9.8.7.6.5.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.8.b.d.0.1.0.0."
                "2.") == "");
    }
}

TEST_CASE("map_errno() works as expected") {
    SECTION("Make sure that 0 maps on mk::NoError") {
        REQUIRE(mk::net::map_errno(0) == mk::NoError());
    }

    SECTION("Make sure that EAGAIN is correctly handled") {
        REQUIRE(mk::net::map_errno(EAGAIN) ==
                mk::net::OperationWouldBlockError());
    }

    SECTION("Make sure that mapped errors map to correct classes") {
#define XX(_code_, _name_, _descr_)                                            \
    {                                                                          \
        auto err_cond = std::make_error_condition(std::errc::_descr_);         \
        int code = err_cond.value();                                           \
        REQUIRE(mk::net::map_errno(code) == mk::net::_name_());                \
    }
        MK_NET_ERRORS_XX
#undef XX
    }

    SECTION("Make sure some errors maps by passing the definition directly") {
        REQUIRE(mk::net::map_errno(EWOULDBLOCK) ==
                mk::net::OperationWouldBlockError());
        REQUIRE(mk::net::map_errno(EINTR) == mk::net::InterruptedError());
        REQUIRE(mk::net::map_errno(ENOBUFS)
                == mk::net::NoBufferSpaceError());
    }

    SECTION("Make sure that unmapped errors map to mk::GenericError") {
        REQUIRE(mk::net::map_errno(ENOENT) == mk::GenericError());
    }
}

TEST_CASE("make_sockaddr() works as expected") {
    SECTION("With numeric port: it deals with invalid address") {
        auto err = mk::net::make_sockaddr("antani", 22, nullptr, nullptr);
        REQUIRE(err == mk::ValueError());
    }

    SECTION("With numeric port: it works with valid IPv4") {
        sockaddr_storage ss = {};
        socklen_t sslen = 0;
        auto err = mk::net::make_sockaddr("8.8.8.8", 22, &ss, &sslen);
        REQUIRE(err == mk::NoError());
        REQUIRE(sslen == sizeof(sockaddr_in));
        sockaddr_in *sin4 = (sockaddr_in *)&ss;
        REQUIRE(sin4->sin_family == AF_INET);
        REQUIRE(sin4->sin_port == htons(22));
        char x[INET_ADDRSTRLEN];
        REQUIRE(inet_ntop(AF_INET, &sin4->sin_addr, x, sizeof(x)) != nullptr);
        REQUIRE(std::string{"8.8.8.8"} == x);
    }

    SECTION("With numeric port: it works with valid IPv6") {
        sockaddr_storage ss = {};
        socklen_t sslen = 0;
        auto err = mk::net::make_sockaddr("fe80::1", 22, &ss, &sslen);
        REQUIRE(err == mk::NoError());
        REQUIRE(sslen == sizeof(sockaddr_in6));
        sockaddr_in6 *sin6 = (sockaddr_in6 *)&ss;
        REQUIRE(sin6->sin6_family == AF_INET6);
        REQUIRE(sin6->sin6_port == htons(22));
        char x[INET6_ADDRSTRLEN];
        REQUIRE(inet_ntop(AF_INET6, &sin6->sin6_addr, x, sizeof(x)) != nullptr);
        REQUIRE(std::string{"fe80::1"} == x);
    }
}
