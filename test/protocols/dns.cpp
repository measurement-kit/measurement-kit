/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

//
// Regression tests for `net/dns.hpp` and `net/dns.cpp`.
//

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <ight/protocols/dns.hpp>

#include <ight/common/check_connectivity.hpp>
#include <ight/common/log.hpp>
#include <ight/common/utils.hpp>

using namespace ight::common::check_connectivity;
using namespace ight::common::libevent;
using namespace ight::common::poller;
using namespace ight::common::settings;
using namespace ight::protocols::dns;

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
    REQUIRE(response.get_failure() == "unknown failure 66");
    REQUIRE(response.get_results().size() == 0);
    REQUIRE(response.get_rtt() == 0.0);
    REQUIRE(response.get_ttl() == 0);
}

//
// TODO: test that Response(...) correctly handles the `libevent` param.
//
// Not urgent because this is implicitly tested by other tests.
//

TEST_CASE(
  "Response(...) only computes RTT when the server actually replied") {
    auto ticks = ight_time_now() - 3.0;

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
    //     https://travis-ci.org/bassosimone/libight/builds/40639940#L634
    //

    Libevent libevent;
    libevent.inet_ntop = [](int, const void *, char *s, socklen_t l) {
        if (s == NULL || l <= 0) {
            throw std::runtime_error("You passed me a bad buffer");
        }
        s[0] = '\0';  // Because it will be converted to std::string
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
                x,                            // value of count
                123, 0.11, NULL, &libevent,
                last_good - 10);              // value of start_from
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
                x,                            // value of count
                123, 0.11, NULL, &libevent,
                last_good - 10);              // value of start_from
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

    Libevent libevent;
    auto called = false;

    libevent.inet_ntop = [&](int, const void *, char *, socklen_t) {
        called = true;  // Make sure this function was called
        return (const char *) NULL;
    };

    auto r = Response(DNS_ERR_NONE, DNS_IPv6_AAAA, 16, 123, 0.11, NULL,
                    &libevent);
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
            continue;  // Skip the known-good cases
        }
        auto r = Response(DNS_ERR_NONE, type, 0, 123, 0.0, NULL);
        REQUIRE(r.get_evdns_status() == DNS_ERR_UNKNOWN);
    }
}

TEST_CASE("Evdns errors are correctly mapped to OONI failures") {

    REQUIRE(Response::map_failure_(DNS_ERR_NONE)
            == "");
    REQUIRE(Response::map_failure_(DNS_ERR_FORMAT)
            == "dns_lookup_error");
    REQUIRE(Response::map_failure_(DNS_ERR_SERVERFAILED)
            == "dns_lookup_error");
    REQUIRE(Response::map_failure_(DNS_ERR_NOTEXIST)
            == "dns_lookup_error");
    REQUIRE(Response::map_failure_(DNS_ERR_NOTIMPL)
            == "dns_lookup_error");
    REQUIRE(Response::map_failure_(DNS_ERR_REFUSED)
            == "dns_lookup_error");

    REQUIRE(Response::map_failure_(DNS_ERR_TRUNCATED)
            == "dns_lookup_error");
    REQUIRE(Response::map_failure_(DNS_ERR_UNKNOWN)
            == "unknown failure 66");
    REQUIRE(Response::map_failure_(DNS_ERR_TIMEOUT)
            == "deferred_timeout_error");
    REQUIRE(Response::map_failure_(DNS_ERR_SHUTDOWN)
            == "unknown failure 68");
    REQUIRE(Response::map_failure_(DNS_ERR_CANCEL)
            == "unknown failure 69");
    REQUIRE(Response::map_failure_(DNS_ERR_NODATA)
            == "dns_lookup_error");

    // Just three random numbers to increase confidence...
    REQUIRE(Response::map_failure_(1024)
            == "unknown failure 1024");
    REQUIRE(Response::map_failure_(1025)
            == "unknown failure 1025");
    REQUIRE(Response::map_failure_(1026)
            == "unknown failure 1026");
}

struct TransparentResponse : public Response {
    using Response::Response;

    int get_code_() {
        return code;
    }
    void set_code_(int v) {
        code = v;
    }

    double get_rtt_() {
        return rtt;
    }
    void set_rtt_(double v) {
        rtt = v;
    }

    int get_ttl_() {
        return ttl;
    }
    void set_ttl_(int v) {
        ttl = v;
    }

    std::vector<std::string> get_results_() {
        return results;
    }
    void set_results_(std::vector<std::string> v) {
        results = v;
    }

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

        r1 = std::move(r2);  /* Move assignment */

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
        REQUIRE(r2.get_code_() == DNS_ERR_NONE);  // copied
        REQUIRE(r2.get_rtt_() == 1.0);            // copied
        REQUIRE(r2.get_ttl_() == 32764);          // copied
        REQUIRE(r2.get_results_().size() == 0);   // move
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

        }(std::move(r2));  /* Move constructor */

        //
        // Note: move semantic is *copy* for integers and the like, which
        // explains why everything but results is copied below.
        //
        REQUIRE(r2.get_code_() == DNS_ERR_NONE);  // copied
        REQUIRE(r2.get_rtt_() == 1.0);            // copied
        REQUIRE(r2.get_ttl_() == 32764);          // copied
        REQUIRE(r2.get_results_().size() == 0);   // moved
    }

}

//
// Request unit test
//

//
// TODO: make sure that Request() correctly handle `dnsb` and `libevent`.
//
// Not urgent because already guaranteed by other tests.
//

// Now testing RequestImpl()

TEST_CASE("Request deals with failing evdns_base_resolve_ipv4") {
    Libevent libevent;

    libevent.evdns_base_resolve_ipv4 = [](evdns_base *, const char *, int,
                                          evdns_callback_type, void *) {
        return (evdns_request *) NULL;
    };

    REQUIRE_THROWS(Request("A", "www.google.com", [](
                                    Response&&) {
        /* nothing */
    }, NULL, &libevent));
}

TEST_CASE("Request deals with failing evdns_base_resolve_ipv6") {
    Libevent libevent;

    libevent.evdns_base_resolve_ipv6 = [](evdns_base *, const char *, int,
                                          evdns_callback_type, void *) {
        return (evdns_request *) NULL;
    };

    REQUIRE_THROWS(Request("AAAA", "github.com", [](
                                    Response&&) {
        /* nothing */
    }, NULL, &libevent));
}

TEST_CASE("Request deals with failing evdns_base_resolve_reverse") {
    Libevent libevent;

    libevent.evdns_base_resolve_reverse = [](evdns_base *,
                                             const struct in_addr *,
                                             int,
                                             evdns_callback_type,
                                             void *) {
        return (evdns_request *) NULL;
    };

    REQUIRE_THROWS(Request("REVERSE_A", "8.8.8.8", [](
                                    Response&&) {
        /* nothing */
    }, NULL, &libevent));
}

TEST_CASE("Request deals with failing evdns_base_resolve_reverse_ipv6") {
    Libevent libevent;

    libevent.evdns_base_resolve_reverse_ipv6 = [](evdns_base *,
                                                  const struct in6_addr *,
                                                  int,
                                                  evdns_callback_type,
                                                  void *) {
        return (evdns_request *) NULL;
    };

    REQUIRE_THROWS(Request("REVERSE_AAAA", "::1", [](
                                    Response&&) {
        /* nothing */
    }, NULL, &libevent));
}

TEST_CASE("Request deals with inet_pton returning 0") {
    Libevent libevent;

    libevent.inet_pton = [](int, const char *, void *) {
        return 0;
    };

    REQUIRE_THROWS(Request("REVERSE_A", "8.8.8.8", [](
                                    Response&&) {
        /* nothing */
    }, NULL, &libevent));

    REQUIRE_THROWS(Request("REVERSE_AAAA", "::1", [](
                                    Response&&) {
        /* nothing */
    }, NULL, &libevent));
}

TEST_CASE("Request deals with inet_pton returning -1") {
    Libevent libevent;

    libevent.inet_pton = [](int, const char *, void *) {
        return -1;
    };

    REQUIRE_THROWS(Request("REVERSE_A", "8.8.8.8", [](
                                    Response&&) {
        /* nothing */
    }, NULL, &libevent));

    REQUIRE_THROWS(Request("REVERSE_AAAA", "::1", [](
                                    Response&&) {
        /* nothing */
    }, NULL, &libevent));
}

TEST_CASE("Request raises if the query is unsupported") {
    REQUIRE_THROWS(Request("PTR", "www.neubot.org",
                   [&](Response&& /*response*/) {
        // nothing
    }));
}

//
// Now testing cancel().
//
// We have two cases: when the request is pending and when the request
// has already returned a value.
//

TEST_CASE("Request::cancel() is idempotent") {

    //
    // Here we only want to see that multiple cancel()s followed
    // by the object being destroyed cause no harm.
    //
    // This is similar to the following test, because in both tests
    // we check that we can get rid of a pending request.
    //

    auto r1 = Request("A", "www.neubot.org", [&](
                               Response&& /*response*/) {
        // nothing
    });

    r1.cancel();
    r1.cancel();
    r1.cancel();
}

TEST_CASE("Request::cancel() is safe when a request is pending") {

    //
    // Note: this test can be run both when the network is down and when
    // the network is up. In both cases, the external Request is deleted
    // before any event happens, and the RequestImpl waits for some
    // event to happen. When the network is down, the event is the timeout
    // of evdns; when the network is up, the event is the DNS reply.
    //

    auto bad_code = false;
    auto called = false;

    Libevent libevent;
    libevent.evdns_reply_hook = [&](int code, char, int, int, void *, void *) {
        called = true;
        if (code != DNS_ERR_NONE && code != DNS_ERR_TIMEOUT) {
            bad_code = true;
        }
        //
        // We need to unblock the loop here, because this test deletes
        // the Request immediately, so we don't have a callback where
        // to report that we are done and we need to break the loop.
        //
        ight_break_loop();
    };

    auto failed = false;
    {
        auto r1 = Request("A", "www.neubot.org", [&](
                                   Response&& /*response*/) {
            //
            // This callback should not be invoked, because RequestImpl
            // should honor its `cancelled` field and therefore should delete
            // itself rather than calling the callback.
            //
            failed = true;
            ight_break_loop();
        }, NULL, &libevent);

    }  // This kills Request, but not the underlying RequestImpl

    ight_loop();
    REQUIRE(!failed);
    REQUIRE(!bad_code);
    REQUIRE(called);
}

//
// TODO: test the case when we receive the message and then free the request.
//
// Not urgent, because it's tested by lots of other tests.
//

struct TransparentRequest : public Request {
    using Request::Request;

    SharedPointer<bool> get_cancelled_() {
        return cancelled;
    }
};

TEST_CASE("Move semantic works for request") {

    SECTION("Move assignment") {

        TransparentRequest r1;

        REQUIRE_THROWS(*r1.get_cancelled_());

        TransparentRequest r2{"A", "www.neubot.org",
                [](Response&&) {
            /* nothing */
        }};
        REQUIRE(*r2.get_cancelled_() == false);

        r1 = std::move(r2);  /* Move assignment */

        REQUIRE(*r1.get_cancelled_() == false);

        REQUIRE_THROWS(*r2.get_cancelled_());
    }

    SECTION("Move constructor") {
        TransparentRequest r2{"A", "www.neubot.org",
                [](Response&&) {
            /* nothing */
        }};
        REQUIRE(*r2.get_cancelled_() == false);

        [](TransparentRequest r1) {
            REQUIRE(*r1.get_cancelled_() == false);

        }(std::move(r2));  /* Move constructor */

        REQUIRE_THROWS(*r2.get_cancelled_());
    }

}

//
// Integration (or regress?) tests for Request.
//
// They generally need connectivity and are automatically skipped if
// we are not connected to the 'Net.
//

TEST_CASE("The system resolver works as expected") {

    //
    // Note: this test also makes sure that we get sensible
    // response fields from the system resolver.
    //

    if (Network::is_down()) {
        return;
    }

    auto failed = false;

    DelayedCall d(10.0, [&](void) {
        failed = true;
        ight_break_loop();
    });

    auto r1 = Request("A", "www.neubot.org", [&](
                               Response&& response) {
        REQUIRE(response.get_reply_authoritative() == "unknown");
        REQUIRE(response.get_evdns_status() == DNS_ERR_NONE);
        REQUIRE(response.get_failure() == "");
        REQUIRE(response.get_results().size() == 1);
        REQUIRE(response.get_results()[0] == "130.192.16.172");
        REQUIRE(response.get_rtt() > 0.0);
        REQUIRE(response.get_ttl() > 0);
        ight_break_loop();
    });
    ight_loop();

    auto r2 = Request("REVERSE_A", "130.192.16.172", [&](
                               Response&& response) {
        REQUIRE(response.get_reply_authoritative() == "unknown");
        REQUIRE(response.get_evdns_status() == DNS_ERR_NONE);
        REQUIRE(response.get_failure() == "");
        REQUIRE(response.get_results().size() == 1);
        REQUIRE(response.get_results()[0] == "server-nexa.polito.it");
        REQUIRE(response.get_rtt() > 0.0);
        REQUIRE(response.get_ttl() > 0);
        ight_break_loop();
    });
    ight_loop();

    auto r3 = Request("AAAA", "ooni.torproject.org", [&](
                               Response&& response) {
        REQUIRE(response.get_reply_authoritative() == "unknown");
        REQUIRE(response.get_evdns_status() == DNS_ERR_NONE);
        REQUIRE(response.get_failure() == "");
        REQUIRE(response.get_results().size() > 0);
        REQUIRE(response.get_rtt() > 0.0);
        REQUIRE(response.get_ttl() > 0);
        auto found = false;
        for (auto address : response.get_results()) {
            if (address == "2001:858:2:2:aabb:0:563b:1e28" ||
                address == "2001:858:2:2:aabb::563b:1e28") {
                found = true;
            }
        }
        REQUIRE(found);
        ight_break_loop();
    });
    ight_loop();

    auto r4 = Request("REVERSE_AAAA", "2001:858:2:2:aabb:0:563b:1e28",
                               [&](Response&& response) {
        REQUIRE(response.get_reply_authoritative() == "unknown");
        REQUIRE(response.get_evdns_status() == DNS_ERR_NONE);
        REQUIRE(response.get_failure() == "");
        REQUIRE(response.get_results().size() == 1);
        REQUIRE(response.get_results()[0] == "nova.torproject.org");
        REQUIRE(response.get_rtt() > 0.0);
        REQUIRE(response.get_ttl() > 0);
        ight_break_loop();
    });
    ight_loop();

    REQUIRE(!failed);
}

class SafeToDeleteRequestInItsOwnCallback {
    Request request;
public:
    SafeToDeleteRequestInItsOwnCallback() {
        request = Request("A", "nexa.polito.it", [this](Response&&) {
            // This assignment should trigger the original request's destructor
            request = Request("AAAA", "nexa.polito.it", [this](Response&&) {
                ight_break_loop();
            });
        });
    }
};

TEST_CASE("It is safe to clear a request in its own callback") {
    if (Network::is_down()) {
        return;
    }
    auto d = SafeToDeleteRequestInItsOwnCallback();
    auto called = 0;
    DelayedCall watchdog(5.0, [&called]() {
        ++called;
        ight_break_loop();
    });
    ight_loop();
    REQUIRE(called == 0);
}

//
// Resolver unit tests.
//

// Now testing: cleanup()

TEST_CASE("Resolver: cleanup works correctly when we have allocated") {
    auto libevent = Libevent();

    auto called = 0;
    libevent.evdns_base_free = [&](evdns_base *p, int f) {
        ::evdns_base_free(p, f);
        called++;
    };

    {
        // Note: call .get_evdns_base() to trigger lazy allocation
        Resolver(Settings(), &libevent)
            .get_evdns_base();
    }

    REQUIRE(called == 1);
}

TEST_CASE("Resolver: cleanup works correctly when we have not allocated") {
    auto libevent = Libevent();

    auto called = 0;
    libevent.evdns_base_free = [&](evdns_base *p, int f) {
        ::evdns_base_free(p, f);
        called++;
    };

    {
        Resolver();
    }

    REQUIRE(called == 0);
}

// Now testing Resolver(...)

TEST_CASE("Resolver: ensure that the constructor does not allocate") {
    auto libevent = Libevent();

    libevent.evdns_base_new = [](event_base *, int) {
        return (evdns_base *) NULL;
    };

    //
    // Basically: if we go through the end we have not allocated because
    // if we try to allocate we fail and the code will raise
    //

    //Resolver();  // How to do this?
    Resolver(Settings(), &libevent);
}

TEST_CASE("Resolver: evdns_base_new failure is correctly handled") {
    auto libevent = Libevent();

    libevent.evdns_base_new = [](event_base *, int) {
        return (evdns_base *) NULL;
    };

    // Note: call .get_evdns_base() to trigger lazy allocation

    // Handle the branch where nameserver is set
    REQUIRE_THROWS(Resolver({
        {"nameserver", "8.8.8.8"}
    }, &libevent).get_evdns_base());

    // Handle the branch using the default nameserver
    REQUIRE_THROWS(Resolver(Settings(),
        &libevent).get_evdns_base());
}

TEST_CASE(
  "Resolver: evdns_base_nameserver_ip_add failure is correctly handled") {
    auto libevent = Libevent();

    libevent.evdns_base_nameserver_ip_add = [](evdns_base *, const char *) {
        return -1;
    };

    // Also make sure that the destructor is called
    auto called = false;
    libevent.evdns_base_free = [&](evdns_base *p, int f) {
        ::evdns_base_free(p, f);
        called = true;
    };

    // Note: call .get_evdns_base() to trigger lazy allocation
    REQUIRE_THROWS(Resolver({
        {"nameserver", "8.8.8.8"}
    }, &libevent).get_evdns_base());

    REQUIRE(called);
}

TEST_CASE("Resolver: evdns_base_set_option failure is correctly handled") {
    auto libevent = Libevent();

    // Also make sure that the destructor is called
    auto called = 0;
    libevent.evdns_base_free = [&](evdns_base *p, int f) {
        ::evdns_base_free(p, f);
        called += 1;
    };

    // Note: call .get_evdns_base() to trigger lazy allocation

    libevent.evdns_base_set_option = [](evdns_base *, const char *opt,
      const char *) {
        if (strcmp(opt, "attempts") == 0) {
            return -1;
        }
        return 0;
    };
    REQUIRE_THROWS(Resolver({
        {"attempts", "1"},
    }, &libevent).get_evdns_base());

    libevent.evdns_base_set_option = [](evdns_base *, const char *opt,
      const char *) {
        if (strcmp(opt, "timeout") == 0) {
            return -1;
        }
        return 0;
    };
    REQUIRE_THROWS(Resolver({
        {"timeout", "1.0"},
    }, &libevent).get_evdns_base());

    libevent.evdns_base_set_option = [](evdns_base *, const char *opt,
      const char *) {
        if (strcmp(opt, "randomize-case") == 0) {
            return -1;
        }
        return 0;
    };
    // Make sure that randomize-case is called in both true and false cases
    REQUIRE_THROWS(Resolver({
        {"randomize_case", "1"},
    }, &libevent).get_evdns_base());
    REQUIRE_THROWS(Resolver({
        {"randomize_case", "0"},
    }, &libevent).get_evdns_base());

    REQUIRE(called == 4);  // twice for randomize-case
}

TEST_CASE("Resolver::get_evdns_base() is idempotent") {
    Resolver reso;
    REQUIRE(reso.get_evdns_base() == reso.get_evdns_base());
}

TEST_CASE("We can override the default timeout") {

    // I need to remember to never run a DNS on that machine :^)
    Resolver reso({
        {"nameserver", "130.192.91.231"},
        {"attempts", "1"},
        {"timeout", "0.5"}
    });

    auto ticks = ight_time_now();
    reso.request("A", "www.neubot.org", [&](Response&& response) {
        REQUIRE(response.get_reply_authoritative() == "unknown");
        REQUIRE(response.get_results().size() == 0);
        REQUIRE(response.get_evdns_status() == DNS_ERR_TIMEOUT);
        REQUIRE(response.get_failure() == "deferred_timeout_error");
        REQUIRE(response.get_ttl() == 0);
        REQUIRE(response.get_rtt() == 0.0);

        auto elapsed = ight_time_now() - ticks;
        // This interval is wide so the unit tests does not fail when
        // we run it using valgrind on slow machines
        REQUIRE(elapsed > 0.3);
        REQUIRE(elapsed < 0.7);

        ight_break_loop();
    });
    ight_loop();
}

TEST_CASE("We can override the default number of tries") {

    // I need to remember to never run a DNS on that machine :^)
    Resolver reso({
        {"nameserver", "130.192.91.231"},
        {"attempts", "2"},
        {"timeout", "0.5"},
    });

    auto ticks = ight_time_now();
    reso.request("A", "www.neubot.org", [&](Response&& response) {
        REQUIRE(response.get_reply_authoritative() == "unknown");
        REQUIRE(response.get_results().size() == 0);
        REQUIRE(response.get_evdns_status() == DNS_ERR_TIMEOUT);
        REQUIRE(response.get_failure() == "deferred_timeout_error");
        REQUIRE(response.get_ttl() == 0);
        REQUIRE(response.get_rtt() == 0.0);

        auto elapsed = ight_time_now() - ticks;
        REQUIRE(elapsed > 0.8);
        REQUIRE(elapsed < 1.2);

        ight_break_loop();
    });
    ight_loop();
}

//
// Intregration (or regress?) tests for Resolver.
//
// They generally need connectivity and are automatically skipped if
// we are not connected to the 'Net.
//

TEST_CASE("The default custom resolver works as expected") {

    if (Network::is_down()) {
        return;
    }

    auto failed = false;

    DelayedCall d(10.0, [&](void) {
        failed = true;
        ight_break_loop();
    });

    Resolver reso;

    reso.request("A", "www.neubot.org", [&](Response&& response) {
        REQUIRE(response.get_reply_authoritative() == "unknown");
        REQUIRE(response.get_evdns_status() == DNS_ERR_NONE);
        REQUIRE(response.get_failure() == "");
        REQUIRE(response.get_results().size() == 1);
        REQUIRE(response.get_results()[0] == "130.192.16.172");
        REQUIRE(response.get_rtt() > 0.0);
        REQUIRE(response.get_ttl() > 0);
        ight_break_loop();
    });
    ight_loop();

    reso.request("REVERSE_A", "130.192.16.172", [&](Response&& response) {
        REQUIRE(response.get_reply_authoritative() == "unknown");
        REQUIRE(response.get_evdns_status() == DNS_ERR_NONE);
        REQUIRE(response.get_failure() == "");
        REQUIRE(response.get_results().size() == 1);
        REQUIRE(response.get_results()[0] == "server-nexa.polito.it");
        REQUIRE(response.get_rtt() > 0.0);
        REQUIRE(response.get_ttl() > 0);
        ight_break_loop();
    });
    ight_loop();

    reso.request("AAAA", "ooni.torproject.org", [&](Response&& response) {
        REQUIRE(response.get_reply_authoritative() == "unknown");
        REQUIRE(response.get_evdns_status() == DNS_ERR_NONE);
        REQUIRE(response.get_failure() == "");
        REQUIRE(response.get_results().size() > 0);
        REQUIRE(response.get_rtt() > 0.0);
        REQUIRE(response.get_ttl() > 0);
        auto found = false;
        for (auto address : response.get_results()) {
            if (address == "2001:858:2:2:aabb:0:563b:1e28" ||
                address == "2001:858:2:2:aabb::563b:1e28") {
                found = true;
            }
        }
        REQUIRE(found);
        ight_break_loop();
    });
    ight_loop();

    reso.request("REVERSE_AAAA", "2001:858:2:2:aabb:0:563b:1e28",
            [&](Response&& response) {
        REQUIRE(response.get_reply_authoritative() == "unknown");
        REQUIRE(response.get_evdns_status() == DNS_ERR_NONE);
        REQUIRE(response.get_failure() == "");
        REQUIRE(response.get_results().size() == 1);
        REQUIRE(response.get_results()[0] == "nova.torproject.org");
        REQUIRE(response.get_rtt() > 0.0);
        REQUIRE(response.get_ttl() > 0);
        ight_break_loop();
    });
    ight_loop();

    REQUIRE(!failed);
}

TEST_CASE("A specific custom resolver works as expected") {

    if (Network::is_down()) {
        return;
    }

    auto failed = false;

    DelayedCall d(10.0, [&](void) {
        failed = true;
        ight_break_loop();
    });

    Resolver reso(Settings({
        {"nameserver", "8.8.4.4"},
    }));

    reso.request("A", "www.neubot.org", [&](Response&& response) {
        REQUIRE(response.get_reply_authoritative() == "unknown");
        REQUIRE(response.get_evdns_status() == DNS_ERR_NONE);
        REQUIRE(response.get_failure() == "");
        REQUIRE(response.get_results().size() == 1);
        REQUIRE(response.get_results()[0] == "130.192.16.172");
        REQUIRE(response.get_rtt() > 0.0);
        REQUIRE(response.get_ttl() > 0);
        ight_break_loop();
    });
    ight_loop();

    reso.request("REVERSE_A", "130.192.16.172", [&](Response&& response) {
        REQUIRE(response.get_reply_authoritative() == "unknown");
        REQUIRE(response.get_evdns_status() == DNS_ERR_NONE);
        REQUIRE(response.get_failure() == "");
        REQUIRE(response.get_results().size() == 1);
        REQUIRE(response.get_results()[0] == "server-nexa.polito.it");
        REQUIRE(response.get_rtt() > 0.0);
        REQUIRE(response.get_ttl() > 0);
        ight_break_loop();
    });
    ight_loop();

    reso.request("AAAA", "ooni.torproject.org", [&](Response&& response) {
        REQUIRE(response.get_reply_authoritative() == "unknown");
        REQUIRE(response.get_evdns_status() == DNS_ERR_NONE);
        REQUIRE(response.get_failure() == "");
        REQUIRE(response.get_results().size() > 0);
        REQUIRE(response.get_rtt() > 0.0);
        REQUIRE(response.get_ttl() > 0);
        auto found = false;
        for (auto address : response.get_results()) {
            if (address == "2001:858:2:2:aabb:0:563b:1e28" ||
                address == "2001:858:2:2:aabb::563b:1e28") {
                found = true;
            }
        }
        REQUIRE(found);
        ight_break_loop();
    });
    ight_loop();

    reso.request("REVERSE_AAAA", "2001:858:2:2:aabb:0:563b:1e28",
            [&](Response&& response) {
        REQUIRE(response.get_reply_authoritative() == "unknown");
        REQUIRE(response.get_evdns_status() == DNS_ERR_NONE);
        REQUIRE(response.get_failure() == "");
        REQUIRE(response.get_results().size() == 1);
        REQUIRE(response.get_results()[0] == "nova.torproject.org");
        REQUIRE(response.get_rtt() > 0.0);
        REQUIRE(response.get_ttl() > 0);
        ight_break_loop();
    });
    ight_loop();

    REQUIRE(!failed);
}

TEST_CASE("If the resolver dies the requests are aborted") {

    //
    // This should work regardless of the network being up or down.
    //

    // I need to remember to never run a DNS on that machine :^)
    auto reso = new Resolver(Settings({
        {"nameserver", "130.192.91.231"},
    }));

    reso->request("A", "www.neubot.org", [&](Response&& response) {
        REQUIRE(response.get_reply_authoritative() == "unknown");
        REQUIRE(response.get_results().size() == 0);
        REQUIRE(response.get_evdns_status() == DNS_ERR_SHUTDOWN);
        REQUIRE(response.get_failure() == "unknown failure 68");
        REQUIRE(response.get_ttl() == 0);
        REQUIRE(response.get_rtt() == 0.0);
        ight_break_loop();
    });

    DelayedCall d1(0.1, [&](void) {
        delete reso;  // Destroy the resolver and see what happens..
                      // in theory the request callback *should* be called
    });

    auto failed = false;
    DelayedCall d2(1.0, [&](void) {
        // This *should not* be called, since the request callback
        // shold be called before than this one.
        failed = true;
        ight_break_loop();
    });

    ight_loop();

    REQUIRE(!failed);
}

TEST_CASE("A request to a nonexistent server times out") {

    //
    // To be fair, this test also makes sense when the network is down,
    // because, when the network is down, the DNS server is nonexistent by
    // definition since we cannot contact it and we receive a timeout.
    //
    // So, I'm commentin out this check:
    //
    //if (Network::is_down()) {
    //    return;
    //}
    //

    // I need to remember to never run a DNS on that machine :^)
    Resolver reso({
        {"nameserver", "130.192.91.231"},
        {"attempts", "1"},
    });
    reso.request("A", "www.neubot.org", [&](Response&& response) {
        REQUIRE(response.get_reply_authoritative() == "unknown");
        REQUIRE(response.get_results().size() == 0);
        REQUIRE(response.get_evdns_status() == DNS_ERR_TIMEOUT);
        REQUIRE(response.get_failure() == "deferred_timeout_error");
        REQUIRE(response.get_ttl() == 0);
        REQUIRE(response.get_rtt() == 0.0);
        ight_break_loop();
    });

    auto failed = false;
    DelayedCall d(10.0, [&](void) {
        failed = true;
        ight_break_loop();
    });

    ight_loop();

    REQUIRE(!failed);
}

TEST_CASE("It is safe to cancel requests in flight") {

    if (Network::is_down()) {
        return;
    }

    //
    // The general idea of this test is to measure the typical RTT with
    // respect to a server and then systematically unschedule pending DNS
    // requests when they are due, to trigger a race between receiving
    // the response and unscheduling the request.
    //
    // This regress test only repeats the process 16 times but I have
    // privately run this test repeating it for about one minute.
    //

    Resolver reso({
        {"nameserver", "8.8.8.8"},
        {"attempts", "1"},
    });

    // Step #1: estimate the average RTT

    auto total = 0.0;
    auto count = 0;
    for (auto i = 0; i < 16; ++i) {
        reso.request("A", "www.neubot.org", [&](Response&& response) {
            if (response.get_evdns_status() == DNS_ERR_NONE) {
                total += response.get_rtt();
                count += 1;
            }
            ight_break_loop();
        });
        ight_loop();
    }
    // We need at lest 8 good responses to compute the average
    // We do not require 16 to tolerate a few losses
    REQUIRE(count > 8);
    auto avgrtt = total / count;

    // Step #2: attempt to unschedule responses when they are due

    //for (;;) {  // only try this at home
    for (auto i = 0; i < 16; ++i) {
        auto r = new Request("A", "www.neubot.org", [&](
                                      Response&& response) {
            auto status_ok = (response.get_evdns_status() == DNS_ERR_CANCEL
                    || response.get_evdns_status() == DNS_ERR_NONE);
            REQUIRE(status_ok);
            // Ignoring all the other fields here
            ight_warn("- break_loop");
            ight_break_loop();
        }, reso.get_evdns_base());
        DelayedCall d(avgrtt, [&](void) {
            ight_warn("- cancel");
            r->cancel();
            ight_break_loop();
        });
        ight_loop();
        delete r;
    }
}

//
// The following is useful to test with tcpdump and/or nc that the
// resolver is actually sending messages to the specified address
// and port and that the desired number of retries it tried by it.
//
// It is currently commented out because I don't know how this
// test could be fully automated (especially in travis-ci).
//
// Anyway, it worked for me!
//

/*
TEST_CASE("Make sure we can override host and number of tries") {
    Resolver reso({
        {"nameserver", "127.0.0.1:5353"},
        {"attempts", "2"},
    });
    reso.request("A", "www.neubot.org", [&](Response response) {
        REQUIRE(response.get_results().size() == 0);
        REQUIRE(response.get_evdns_status() == DNS_ERR_TIMEOUT);
        // Assuming all the other fields are OK
        ight_break_loop();
    });
    ight_loop();
}
*/
