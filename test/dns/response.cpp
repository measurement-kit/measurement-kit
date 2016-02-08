// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include <measurement_kit/dns.hpp>
#include <measurement_kit/common.hpp>

#include "src/common/utils.hpp"
#include "src/common/delayed_call.hpp"

using namespace mk;
using namespace mk::dns;

//
// Response unit tests.
//

TEST_CASE("The default Response() constructor sets sensible values") {
    auto response = Response();
    //
    // Not everything is empty, but an error is set (DNS_ERR_UNKNOWN)
    // and, hey, I think this is acceptable.
    //
    REQUIRE(response.get_reply_authoritative() == "unknown");
    REQUIRE(response.get_evdns_status() == DNS_ERR_UNKNOWN);
    REQUIRE(response.get_results().size() == 0);
    REQUIRE(response.get_rtt() == 0.0);
    REQUIRE(response.get_ttl() == 0);
}

//
// TODO: test that Response(...) correctly handles the `libs` param.
//
// Not urgent because this is implicitly tested by other tests.
//

TEST_CASE("Response(...) only computes RTT when the server actually replied") {
    auto ticks = mk::time_now() - 3.0;

    for (auto code = 0; code < 128; ++code) {
        auto r = Response(code, DNS_IPv4_A, 0, 123, ticks, NULL);
        switch (code) {
        case DNS_ERR_NONE:
        case DNS_ERR_FORMAT:
        case DNS_ERR_SERVERFAILED:
        case DNS_ERR_NOTEXIST:
        case DNS_ERR_NOTIMPL:
        case DNS_ERR_REFUSED:
        case DNS_ERR_TRUNCATED:
        case DNS_ERR_NODATA:
            REQUIRE(r.get_rtt() > 0);
            break;
        default:
            REQUIRE(r.get_rtt() == 0);
            break;
        }
    }
}

//
// TODO: if code is error, we don't fill results.
//
// Not urgent because implicitly tested by other tests.
//

//
// TODO: if the result is PTR, we correctly set the value.
//
// Not urgent because implicitly tested by other tests.
//

//
// TODO: if the result is A or AAAA, we correctly fill the vector.
//
// Not urgent because implicitly tested by other tests.
//

TEST_CASE("Response(...) constructor is robust against overflow") {

    //
    // Mock inet_ntop() so that we don't pass it a NULL pointer. This causes
    // crashes on Linux (but not on MacOSX), see:
    //
    //     https://travis-ci.org/measurement-kit/measurement-kit/builds/40639940#L634
    //

    Libs libs;
    libs.inet_ntop = [](int, const void *, char *s, socklen_t l) {
        if (s == NULL || l <= 0) {
            throw std::runtime_error("You passed me a bad buffer");
        }
        s[0] = '\0'; // Because it will be converted to std::string
        return s;
    };

    //
    // The code we are testing is functionally equivalent to the
    // following piece of code
    //
    //     for (auto i = start_from; i < count; ++i) {
    //
    //         if (i > INT_MAX / size) {
    //             code = DNS_ERR_UNKNOWN;
    //             break;
    //         }
    //
    //         int off = i * size;  // This must not overflow.
    //
    // The maximum value of `i` is `count - 1`. To find the maximum
    // acceptable value of `count` we must solve this
    //
    //     count - 1 > INT_MAX / size
    //
    // Leading to
    //
    //     count > INT_MAX / size + 1
    //
    // Therefore, the last accepted value before breaking out of
    // the above loop shall be
    //
    //     last_good = INT_MAX / size + 1
    //
    // and the we break out of the loop when `i == last_good + 1`.
    //
    // Note: here, we set `start_from` to `last_good - 10` for test
    // speed reasons, but in real code that value is zero.
    //
    // Also, `size` is 4 for IPv4 and 16 for IPv6.
    //

    SECTION("IPv4: the overflow check behaves correctly") {
        auto last_good = INT_MAX / 4 + 1;
        for (auto x = last_good - 5; x <= last_good + 5; x += 1) {
            auto r = Response(DNS_ERR_NONE, DNS_IPv4_A,
                              x, // value of count
                              123, 0.11, NULL, Logger::global(), &libs,
                              last_good - 10); // value of start_from
            if (x <= last_good) {
                REQUIRE(r.get_evdns_status() == DNS_ERR_NONE);
                REQUIRE(r.get_results().size() > 0);
            } else {
                REQUIRE(r.get_evdns_status() == DNS_ERR_UNKNOWN);
                REQUIRE(r.get_results().size() == 0);
            }
        }
    }

    SECTION("IPv4: zero records are correctly handled by the overflow check") {
        auto r = Response(DNS_ERR_NONE, DNS_IPv4_A, 0, 123, 0.11, NULL);
        REQUIRE(r.get_evdns_status() == DNS_ERR_NONE);
        REQUIRE(r.get_results().size() == 0);
    }

    SECTION("IPv6: the overflow check behaves correctly") {
        auto last_good = INT_MAX / 16 + 1;
        for (auto x = last_good - 5; x <= last_good + 5; x += 1) {
            auto r = Response(DNS_ERR_NONE, DNS_IPv6_AAAA,
                              x, // value of count
                              123, 0.11, NULL, Logger::global(), &libs,
                              last_good - 10); // value of start_from
            if (x <= last_good) {
                REQUIRE(r.get_evdns_status() == DNS_ERR_NONE);
                REQUIRE(r.get_results().size() > 0);
            } else {
                REQUIRE(r.get_evdns_status() == DNS_ERR_UNKNOWN);
                REQUIRE(r.get_results().size() == 0);
            }
        }
    }

    SECTION("IPv6: zero records are correctly handled by the overflow check") {
        auto r = Response(DNS_ERR_NONE, DNS_IPv6_AAAA, 0, 123, 0.11, NULL);
        REQUIRE(r.get_evdns_status() == DNS_ERR_NONE);
        REQUIRE(r.get_results().size() == 0);
    }
}

TEST_CASE("Response(...) deals with unlikely inet_ntop failures") {

    Libs libs;
    auto called = false;

    libs.inet_ntop = [&](int, const void *, char *, socklen_t) {
        called = true; // Make sure this function was called
        return (const char *)NULL;
    };

    auto r = Response(DNS_ERR_NONE, DNS_IPv6_AAAA, 16, 123, 0.11, NULL,
                      Logger::global(), &libs);
    REQUIRE(r.get_evdns_status() == DNS_ERR_UNKNOWN);
    REQUIRE(r.get_results().size() == 0);

    REQUIRE(called);
}

TEST_CASE("If the response type is invalid Response sets an error") {
    for (char type = 0; type < 16; ++type) {
        switch (type) {
        case DNS_IPv4_A:
        case DNS_IPv6_AAAA:
        case DNS_PTR:
            continue; // Skip the known-good cases
        }
        auto r = Response(DNS_ERR_NONE, type, 0, 123, 0.0, NULL);
        REQUIRE(r.get_evdns_status() == DNS_ERR_UNKNOWN);
    }
}

struct TransparentResponse : public Response {
    using Response::Response;

    int get_code_() { return code; }
    void set_code_(int v) { code = v; }

    double get_rtt_() { return rtt; }
    void set_rtt_(double v) { rtt = v; }

    int get_ttl_() { return ttl; }
    void set_ttl_(int v) { ttl = v; }

    std::vector<std::string> get_results_() { return results; }
    void set_results_(std::vector<std::string> v) { results = v; }
};

TEST_CASE("Move semantic works for response") {

    std::vector<std::string> vector{
        "antani", "blinda",
    };

    SECTION("Move assignment") {

        TransparentResponse r1;

        REQUIRE(r1.get_code_() == DNS_ERR_UNKNOWN);
        REQUIRE(r1.get_rtt_() == 0.0);
        REQUIRE(r1.get_ttl_() == 0);
        REQUIRE(r1.get_results_().size() == 0);

        TransparentResponse r2;

        r2.set_code_(DNS_ERR_NONE);
        r2.set_rtt_(1.0);
        r2.set_ttl_(32764);
        r2.set_results_(vector);

        r1 = std::move(r2); /* Move assignment */

        REQUIRE(r1.get_code_() == DNS_ERR_NONE);
        REQUIRE(r1.get_rtt_() == 1.0);
        REQUIRE(r1.get_ttl_() == 32764);
        REQUIRE(r1.get_results_().size() == 2);
        REQUIRE(r1.get_results_()[0] == "antani");
        REQUIRE(r1.get_results_()[1] == "blinda");

        //
        // Note: move semantic is *copy* for integers and the like, which
        // explains why everything but results is copied below.
        //
        REQUIRE(r2.get_code_() == DNS_ERR_NONE); // copied
        REQUIRE(r2.get_rtt_() == 1.0);           // copied
        REQUIRE(r2.get_ttl_() == 32764);         // copied
        REQUIRE(r2.get_results_().size() == 0);  // move
    }

    SECTION("Move constructor") {

        TransparentResponse r2;
        r2.set_code_(DNS_ERR_NONE);
        r2.set_rtt_(1.0);
        r2.set_ttl_(32764);
        r2.set_results_(vector);

        [](TransparentResponse r1) {
            REQUIRE(r1.get_code_() == DNS_ERR_NONE);
            REQUIRE(r1.get_rtt_() == 1.0);
            REQUIRE(r1.get_ttl_() == 32764);
            REQUIRE(r1.get_results_().size() == 2);
            REQUIRE(r1.get_results_()[0] == "antani");
            REQUIRE(r1.get_results_()[1] == "blinda");

        }(std::move(r2)); /* Move constructor */

        //
        // Note: move semantic is *copy* for integers and the like, which
        // explains why everything but results is copied below.
        //
        REQUIRE(r2.get_code_() == DNS_ERR_NONE); // copied
        REQUIRE(r2.get_rtt_() == 1.0);           // copied
        REQUIRE(r2.get_ttl_() == 32764);         // copied
        REQUIRE(r2.get_results_().size() == 0);  // moved
    }
}
