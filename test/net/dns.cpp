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

#include "net/dns.hpp"

#include "common/log.h"
#include "common/utils.h"

//
// Class used to check whether the network is up.
//
// If the network is down, we skip some tests. To add such check to
// a test, you simply need to add:
//
//     if (Network::is_down()) {
//         return;
//     }
//
// This class reports that the network is up if 8.8.4.4 works and returns
// a valid IPv4 address for github.com. To reach 8.8.4.4 we use evdns, which
// we assume to be working. If evdns is broken, 8.8.4.4 is not reachable or
// github.com is no longer available, this class reports that the network
// is down even if it is not actually down. All these three conditions are
// quite unlikely, IMO, so this code should be robust enough.
//

class Network {
    event_base *evbase = NULL;
    evdns_base *dnsbase = NULL;
    bool is_up = false;

    void cleanup(void) {  // Idempotent cleanup function
        if (dnsbase != NULL) {
            evdns_base_free(dnsbase, 0);
            dnsbase = NULL;
        }
        if (evbase != NULL) {
            event_base_free(evbase);
            evbase = NULL;
        }
    }

    static void dns_callback(int result, char type, int count, int ttl,
                             void *addresses, void *opaque) {

        auto that = static_cast<Network *>(opaque);

        // Suppress "unused variable" warnings
        (void) type;
        (void) count;
        (void) ttl;
        (void) addresses;

        that->is_up = (result == DNS_ERR_NONE);

        if (event_base_loopbreak(that->evbase) != 0) {
            throw std::runtime_error("Cannot exit from event loop");
        }
    }

    Network(void) {
        if ((evbase = event_base_new()) == NULL) {
            cleanup();
            throw std::bad_alloc();
        }

        if ((dnsbase = evdns_base_new(evbase, 0)) == NULL) {
            cleanup();
            throw std::bad_alloc();
        }
        if (evdns_base_nameserver_ip_add(dnsbase, "8.8.4.4") != 0) {
            cleanup();
            throw std::runtime_error("cannot add IP address");
        }

        if (evdns_base_resolve_ipv4(dnsbase, "github.com",
                                    DNS_QUERY_NO_SEARCH,
                                    dns_callback, this) == NULL) {
            cleanup();
            throw std::runtime_error("cannot resolve 'github.com'");
        }

        if (event_base_dispatch(evbase) != 0) {
            cleanup();
            throw std::runtime_error("event_base_dispatch() failed");
        }

        cleanup();

        if (!is_up) {
            std::clog << "Network is down: skipping network related tests"
                      << std::endl;
        }
    }

    ~Network(void) {
        cleanup();
    }

    Network(Network& /*other*/) = delete;
    Network& operator=(Network& /*other*/) = delete;
    Network(Network&& /*other*/) = delete;
    Network& operator=(Network&& /*other*/) = delete;

public:
    static bool is_down(void) {
        static Network singleton;
        return !singleton.is_up;
    }
};

//
// DNSResponse unit tests.
//
// Following the order in which methods are declared in `net/dns.hpp`.
//

TEST_CASE("The default DNSResponse() constructor sets sensible values") {
    auto response = ight::DNSResponse();
    REQUIRE(response.get_query_name() == "");
    REQUIRE(response.get_query_type() == "");
    REQUIRE(response.get_query_class() == "");
    //
    // Not everything is empty, but an error is set (DNS_ERR_UNKNOWN)
    // and, hey, I think this is acceptable.
    //
    REQUIRE(response.get_reply_authoritative() == "unknown");
    REQUIRE(response.get_resolver()[0] == "<default>");
    REQUIRE(response.get_resolver()[1] == "53");
    REQUIRE(response.get_evdns_status() == DNS_ERR_UNKNOWN);
    REQUIRE(response.get_failure() == "unknown failure 66");
    REQUIRE(response.get_results().size() == 0);
    REQUIRE(response.get_rtt() == 0.0);
    REQUIRE(response.get_ttl() == 0);
}

//
// TODO: test that DNSResponse(...) correctly handles the `libevent` param.
//
// Not urgent because this is implicitly tested by other tests.
//

TEST_CASE(
  "DNSResponse(...) only computes RTT when the server actually replied") {
    auto ticks = ight_time_now() - 3.0;

    for (auto code = 0; code < 128; ++code) {
        auto r = ight::DNSResponse("www.google.com", "AAAA", "IN", "8.8.8.8",
            code, DNS_IPv4_A, 0, 123, ticks, NULL);
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

TEST_CASE("DNSResponse(...) constructor is robust against overflow") {

    //
    // Mock inet_ntop() so that we don't pass it a NULL pointer. This causes
    // crashes on Linux (but not on MacOSX), see:
    //
    //     https://travis-ci.org/bassosimone/libight/builds/40639940#L634
    //

    IghtLibevent libevent;
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
            auto r = ight::DNSResponse("www.google.com", "A", "IN", "8.8.8.8",
                DNS_ERR_NONE, DNS_IPv4_A,
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
        auto r = ight::DNSResponse("www.google.com", "A", "IN", "8.8.8.8",
            DNS_ERR_NONE, DNS_IPv4_A, 0, 123, 0.11, NULL);
        REQUIRE(r.get_evdns_status() == DNS_ERR_NONE);
        REQUIRE(r.get_results().size() == 0);
    }

    SECTION("IPv6: the overflow check behaves correctly") {
        auto last_good = INT_MAX / 16 + 1;
        for (auto x = last_good - 5; x <= last_good + 5; x += 1) {
            auto r = ight::DNSResponse("www.google.com", "AAAA", "IN", "::1",
                DNS_ERR_NONE, DNS_IPv6_AAAA,
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
        auto r = ight::DNSResponse("www.google.com", "AAAA", "IN", "::1",
            DNS_ERR_NONE, DNS_IPv6_AAAA, 0, 123, 0.11, NULL);
        REQUIRE(r.get_evdns_status() == DNS_ERR_NONE);
        REQUIRE(r.get_results().size() == 0);
    }
}

TEST_CASE("DNSResponse(...) deals with unlikely inet_ntop failures") {

    IghtLibevent libevent;
    auto called = false;

    libevent.inet_ntop = [&](int, const void *, char *, socklen_t) {
        called = true;  // Make sure this function was called
        return (const char *) NULL;
    };

    auto r = ight::DNSResponse("www.google.com", "AAAA", "IN", "::1",
        DNS_ERR_NONE, DNS_IPv6_AAAA, 16, 123, 0.11, NULL, &libevent);
    REQUIRE(r.get_evdns_status() == DNS_ERR_UNKNOWN);
    REQUIRE(r.get_results().size() == 0);

    REQUIRE(called);
}

TEST_CASE("If the response type is invalid DNSResponse sets an error") {
    for (char type = 0; type < 16; ++type) {
        switch (type) {
        case DNS_IPv4_A:
        case DNS_IPv6_AAAA:
        case DNS_PTR:
            continue;  // Skip the known-good cases
        }
        auto r = ight::DNSResponse("www.google.com", "AAAA", "IN", "::1",
            DNS_ERR_NONE, type, 0, 123, 0.0, NULL);
        REQUIRE(r.get_evdns_status() == DNS_ERR_UNKNOWN);
    }
}

TEST_CASE("Evdns errors are correctly mapped to OONI failures") {

    REQUIRE(ight::DNSResponse::map_failure_(DNS_ERR_NONE)
            == "");
    REQUIRE(ight::DNSResponse::map_failure_(DNS_ERR_FORMAT)
            == "dns_lookup_error");
    REQUIRE(ight::DNSResponse::map_failure_(DNS_ERR_SERVERFAILED)
            == "dns_lookup_error");
    REQUIRE(ight::DNSResponse::map_failure_(DNS_ERR_NOTEXIST)
            == "dns_lookup_error");
    REQUIRE(ight::DNSResponse::map_failure_(DNS_ERR_NOTIMPL)
            == "dns_lookup_error");
    REQUIRE(ight::DNSResponse::map_failure_(DNS_ERR_REFUSED)
            == "dns_lookup_error");

    REQUIRE(ight::DNSResponse::map_failure_(DNS_ERR_TRUNCATED)
            == "dns_lookup_error");
    REQUIRE(ight::DNSResponse::map_failure_(DNS_ERR_UNKNOWN)
            == "unknown failure 66");
    REQUIRE(ight::DNSResponse::map_failure_(DNS_ERR_TIMEOUT)
            == "deferred_timeout_error");
    REQUIRE(ight::DNSResponse::map_failure_(DNS_ERR_SHUTDOWN)
            == "unknown failure 68");
    REQUIRE(ight::DNSResponse::map_failure_(DNS_ERR_CANCEL)
            == "unknown failure 69");
    REQUIRE(ight::DNSResponse::map_failure_(DNS_ERR_NODATA)
            == "dns_lookup_error");

    // Just three random numbers to increase confidence...
    REQUIRE(ight::DNSResponse::map_failure_(1024)
            == "unknown failure 1024");
    REQUIRE(ight::DNSResponse::map_failure_(1025)
            == "unknown failure 1025");
    REQUIRE(ight::DNSResponse::map_failure_(1026)
            == "unknown failure 1026");
}

//
// DNSRequest unit test
//

//
// TODO: make sure that DNSRequest() correctly handle `dnsb` and `libevent`.
//
// Not urgent because already guaranteed by other tests.
//

// Now testing DNSRequestImpl()

TEST_CASE("DNSRequest deals with failing evdns_base_resolve_ipv4") {
    IghtLibevent libevent;

    libevent.evdns_base_resolve_ipv4 = [](evdns_base *, const char *, int,
                                          evdns_callback_type, void *) {
        return (evdns_request *) NULL;
    };

    REQUIRE_THROWS(ight::DNSRequest("A", "www.google.com", [](
                                    ight::DNSResponse&&) {
        /* nothing */
    }, NULL, "", &libevent));
}

TEST_CASE("DNSRequest deals with failing evdns_base_resolve_ipv6") {
    IghtLibevent libevent;

    libevent.evdns_base_resolve_ipv6 = [](evdns_base *, const char *, int,
                                          evdns_callback_type, void *) {
        return (evdns_request *) NULL;
    };

    REQUIRE_THROWS(ight::DNSRequest("AAAA", "github.com", [](
                                    ight::DNSResponse&&) {
        /* nothing */
    }, NULL, "", &libevent));
}

TEST_CASE("DNSRequest deals with failing evdns_base_resolve_reverse") {
    IghtLibevent libevent;

    libevent.evdns_base_resolve_reverse = [](evdns_base *,
                                             const struct in_addr *,
                                             int,
                                             evdns_callback_type,
                                             void *) {
        return (evdns_request *) NULL;
    };

    REQUIRE_THROWS(ight::DNSRequest("REVERSE_A", "8.8.8.8", [](
                                    ight::DNSResponse&&) {
        /* nothing */
    }, NULL, "", &libevent));
}

TEST_CASE("DNSRequest deals with failing evdns_base_resolve_reverse_ipv6") {
    IghtLibevent libevent;

    libevent.evdns_base_resolve_reverse_ipv6 = [](evdns_base *,
                                                  const struct in6_addr *,
                                                  int,
                                                  evdns_callback_type,
                                                  void *) {
        return (evdns_request *) NULL;
    };

    REQUIRE_THROWS(ight::DNSRequest("REVERSE_AAAA", "::1", [](
                                    ight::DNSResponse&&) {
        /* nothing */
    }, NULL, "", &libevent));
}

TEST_CASE("DNSRequest deals with inet_pton returning 0") {
    IghtLibevent libevent;

    libevent.inet_pton = [](int, const char *, void *) {
        return 0;
    };

    REQUIRE_THROWS(ight::DNSRequest("REVERSE_A", "8.8.8.8", [](
                                    ight::DNSResponse&&) {
        /* nothing */
    }, NULL, "", &libevent));

    REQUIRE_THROWS(ight::DNSRequest("REVERSE_AAAA", "::1", [](
                                    ight::DNSResponse&&) {
        /* nothing */
    }, NULL, "", &libevent));
}

TEST_CASE("DNSRequest deals with inet_pton returning -1") {
    IghtLibevent libevent;

    libevent.inet_pton = [](int, const char *, void *) {
        return -1;
    };

    REQUIRE_THROWS(ight::DNSRequest("REVERSE_A", "8.8.8.8", [](
                                    ight::DNSResponse&&) {
        /* nothing */
    }, NULL, "", &libevent));

    REQUIRE_THROWS(ight::DNSRequest("REVERSE_AAAA", "::1", [](
                                    ight::DNSResponse&&) {
        /* nothing */
    }, NULL, "", &libevent));
}

TEST_CASE("DNSRequest raises if the query is unsupported") {
    REQUIRE_THROWS(ight::DNSRequest("PTR", "www.neubot.org",
                   [&](ight::DNSResponse&& /*response*/) {
        // nothing
    }));
}

//
// Now testing cancel().
//
// We have two cases: when the request is pending and when the request
// has already returned a value.
//

TEST_CASE("DNSRequest::cancel() is idempotent") {

    //
    // Here we only want to see that multiple cancel()s followed
    // by the object being destroyed cause no harm.
    //
    // This is similar to the following test, because in both tests
    // we check that we can get rid of a pending request.
    //

    auto r1 = ight::DNSRequest("A", "www.neubot.org", [&](
                               ight::DNSResponse&& /*response*/) {
        // nothing
    });

    r1.cancel();
    r1.cancel();
    r1.cancel();
}

TEST_CASE("DNSRequest::cancel() is safe when a request is pending") {

    //
    // Note: this test can be run both when the network is down and when
    // the network is up. In both cases, the external DNSRequest is deleted
    // before any event happens, and the DNSRequestImpl waits for some
    // event to happen. When the network is down, the event is the timeout
    // of evdns; when the network is up, the event is the DNS reply.
    //

    auto bad_code = false;
    auto called = false;

    IghtLibevent libevent;
    libevent.evdns_reply_hook = [&](int code, char, int, int, void *, void *) {
        called = true;
        if (code != DNS_ERR_NONE && code != DNS_ERR_TIMEOUT) {
            bad_code = true;
        }
        //
        // We need to unblock the loop here, because this test deletes
        // the DNSRequest immediately, so we don't have a callback where
        // to report that we are done and we need to break the loop.
        //
        ight_break_loop();
    };

    auto failed = false;
    {
        auto r1 = ight::DNSRequest("A", "www.neubot.org", [&](
                                   ight::DNSResponse&& /*response*/) {
            //
            // This callback should not be invoked, because DNSRequestImpl
            // should honor its `cancelled` field and therefore should delete
            // itself rather than calling the callback.
            //
            failed = true;
            ight_break_loop();
        }, NULL, "", &libevent);

    }  // This kills DNSRequest, but not the underlying DNSRequestImpl

    ight_loop();
    REQUIRE(!failed);
    REQUIRE(!bad_code);
    REQUIRE(called);
}

//
// TODO: test the case when we receive the message and then free the request.
//
// Not urgent, because it's tested by losts of other tests.
//

//
// Integration (or regress?) tests for DNSRequest.
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

    auto d = IghtDelayedCall(10.0, [&](void) {
        failed = true;
        ight_break_loop();
    });

    auto r1 = ight::DNSRequest("A", "www.neubot.org", [&](
                               ight::DNSResponse&& response) {
        REQUIRE(response.get_query_name() == "www.neubot.org");
        REQUIRE(response.get_query_type() == "A");
        REQUIRE(response.get_query_class() == "IN");
        REQUIRE(response.get_reply_authoritative() == "unknown");
        REQUIRE(response.get_resolver()[0] == "<default>");
        REQUIRE(response.get_resolver()[1] == "53");
        REQUIRE(response.get_evdns_status() == DNS_ERR_NONE);
        REQUIRE(response.get_failure() == "");
        REQUIRE(response.get_results().size() == 1);
        REQUIRE(response.get_results()[0] == "130.192.16.172");
        REQUIRE(response.get_rtt() > 0.0);
        REQUIRE(response.get_ttl() > 0);
        ight_break_loop();
    });
    ight_loop();

    auto r2 = ight::DNSRequest("REVERSE_A", "130.192.16.172", [&](
                               ight::DNSResponse&& response) {
        REQUIRE(response.get_query_name() == "130.192.16.172");
        REQUIRE(response.get_query_type() == "PTR");
        REQUIRE(response.get_query_class() == "IN");
        REQUIRE(response.get_reply_authoritative() == "unknown");
        REQUIRE(response.get_resolver()[0] == "<default>");
        REQUIRE(response.get_resolver()[1] == "53");
        REQUIRE(response.get_evdns_status() == DNS_ERR_NONE);
        REQUIRE(response.get_failure() == "");
        REQUIRE(response.get_results().size() == 1);
        REQUIRE(response.get_results()[0] == "server-nexa.polito.it");
        REQUIRE(response.get_rtt() > 0.0);
        REQUIRE(response.get_ttl() > 0);
        ight_break_loop();
    });
    ight_loop();

    auto r3 = ight::DNSRequest("AAAA", "ooni.torproject.org", [&](
                               ight::DNSResponse&& response) {
        REQUIRE(response.get_query_name() == "ooni.torproject.org");
        REQUIRE(response.get_query_type() == "AAAA");
        REQUIRE(response.get_query_class() == "IN");
        REQUIRE(response.get_reply_authoritative() == "unknown");
        REQUIRE(response.get_resolver()[0] == "<default>");
        REQUIRE(response.get_resolver()[1] == "53");
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

    auto r4 = ight::DNSRequest("REVERSE_AAAA", "2001:858:2:2:aabb:0:563b:1e28",
                               [&](ight::DNSResponse&& response) {
        REQUIRE(response.get_query_name() == "2001:858:2:2:aabb:0:563b:1e28");
        REQUIRE(response.get_query_type() == "PTR");
        REQUIRE(response.get_query_class() == "IN");
        REQUIRE(response.get_reply_authoritative() == "unknown");
        REQUIRE(response.get_resolver()[0] == "<default>");
        REQUIRE(response.get_resolver()[1] == "53");
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

//
// DNSResolver unit tests.
//

// Now testing: cleanup()

TEST_CASE("DNSResolver: cleanup works correctly when we have allocated") {
    auto libevent = IghtLibevent();

    auto called = 0;
    libevent.evdns_base_free = [&](evdns_base *p, int f) {
        ::evdns_base_free(p, f);
        called++;
    };

    {
        ight::DNSResolver("", 0, NULL, &libevent);
    }

    REQUIRE(called == 1);
}

TEST_CASE("DNSResolver: cleanup works correctly when we have not allocated") {
    auto libevent = IghtLibevent();

    auto called = 0;
    libevent.evdns_base_free = [&](evdns_base *p, int f) {
        ::evdns_base_free(p, f);
        called++;
    };

    // Test many combination of default values:
    ight::DNSResolver();
    ight::DNSResolver("");
    ight::DNSResolver("", 0);
    ight::DNSResolver("", 0, NULL);
    ight::DNSResolver("", 0, NULL, NULL);

    REQUIRE(called == 0);
}

// Now testing DNSResolver(...)

//
// TODO: test that we do nothing in DNSResolver().
//
// Also implicitly tested elsewhere.
//

TEST_CASE("DNSResolver: evdns_base_new failure is correctly handled") {
    auto libevent = IghtLibevent();

    libevent.evdns_base_new = [](event_base *, int) {
        return (evdns_base *) NULL;
    };

    // Handle the branch where nameserver is set
    REQUIRE_THROWS(ight::DNSResolver("8.8.8.8", 0, NULL, &libevent));

    // Handle the branch using the default nameserver
    REQUIRE_THROWS(ight::DNSResolver("", 0, NULL, &libevent));
}

TEST_CASE(
  "DNSResolver: evdns_base_nameserver_ip_add failure is correctly handled") {
    auto libevent = IghtLibevent();

    libevent.evdns_base_nameserver_ip_add = [](evdns_base *, const char *) {
        return -1;
    };

    // Also make sure that the destructor is called
    auto called = false;
    libevent.evdns_base_free = [&](evdns_base *p, int f) {
        ::evdns_base_free(p, f);
        called = true;
    };

    REQUIRE_THROWS(ight::DNSResolver("8.8.8.8", 0, NULL, &libevent));
    REQUIRE(called);
}

TEST_CASE("DNSResolver: evdns_base_set_option failure is correctly handled") {
    auto libevent = IghtLibevent();

    libevent.evdns_base_set_option = [](evdns_base *, const char *,
      const char *) {
        return -1;
    };

    // Also make sure that the destructor is called
    auto called = false;
    libevent.evdns_base_free = [&](evdns_base *p, int f) {
        ::evdns_base_free(p, f);
        called = true;
    };

    REQUIRE_THROWS(ight::DNSResolver("", 1, NULL, &libevent));
    REQUIRE(called);
}

TEST_CASE("DNSResolver: get_evdns_base behaves correctly") {

    // Cases in which it must return the global evdns_base:

    {
        auto r = ight::DNSResolver();
        REQUIRE(r.get_evdns_base() == ight_get_global_evdns_base());
    }

    {
        auto r = ight::DNSResolver("");
        REQUIRE(r.get_evdns_base() == ight_get_global_evdns_base());
    }

    {
        auto r = ight::DNSResolver("", 0);
        REQUIRE(r.get_evdns_base() == ight_get_global_evdns_base());
    }

    {
        auto r = ight::DNSResolver("", 0, NULL);
        REQUIRE(r.get_evdns_base() == ight_get_global_evdns_base());
    }

    {
        auto r = ight::DNSResolver("", 0, NULL, NULL);
        REQUIRE(r.get_evdns_base() == ight_get_global_evdns_base());
    }

    // Cases in which it must return a private evdns_base:

    {
        auto r = ight::DNSResolver("8.8.8.8", 0, NULL, NULL);
        REQUIRE(r.get_evdns_base() != ight_get_global_evdns_base());
    }

    {
        auto r = ight::DNSResolver("", 1, NULL, NULL);
        REQUIRE(r.get_evdns_base() != ight_get_global_evdns_base());
    }

    {
        IghtPoller p;
        auto r = ight::DNSResolver("", 0, &p, NULL);
        REQUIRE(r.get_evdns_base() != ight_get_global_evdns_base());
    }

    {
        IghtLibevent libevent;
        auto r = ight::DNSResolver("", 0, NULL, &libevent);
        REQUIRE(r.get_evdns_base() != ight_get_global_evdns_base());
    }
}

//
// Intregration (or regress?) tests for DNSResolver.
//
// They generally need connectivity and are automatically skipped if
// we are not connected to the 'Net.
//

TEST_CASE("The default custom resolver works as expected") {

    if (Network::is_down()) {
        return;
    }

    auto failed = false;

    auto d = IghtDelayedCall(10.0, [&](void) {
        failed = true;
        ight_break_loop();
    });

    auto reso = ight::DNSResolver();

    auto r1 = reso.request("A", "www.neubot.org", [&](
                           ight::DNSResponse&& response) {
        REQUIRE(response.get_query_name() == "www.neubot.org");
        REQUIRE(response.get_query_type() == "A");
        REQUIRE(response.get_query_class() == "IN");
        REQUIRE(response.get_reply_authoritative() == "unknown");
        REQUIRE(response.get_resolver()[0] == "<default>");
        REQUIRE(response.get_resolver()[1] == "53");
        REQUIRE(response.get_evdns_status() == DNS_ERR_NONE);
        REQUIRE(response.get_failure() == "");
        REQUIRE(response.get_results().size() == 1);
        REQUIRE(response.get_results()[0] == "130.192.16.172");
        REQUIRE(response.get_rtt() > 0.0);
        REQUIRE(response.get_ttl() > 0);
        ight_break_loop();
    });
    ight_loop();

    auto r2 = reso.request("REVERSE_A", "130.192.16.172", [&](
                           ight::DNSResponse&& response) {
        REQUIRE(response.get_query_name() == "130.192.16.172");
        REQUIRE(response.get_query_type() == "PTR");
        REQUIRE(response.get_query_class() == "IN");
        REQUIRE(response.get_reply_authoritative() == "unknown");
        REQUIRE(response.get_resolver()[0] == "<default>");
        REQUIRE(response.get_resolver()[1] == "53");
        REQUIRE(response.get_evdns_status() == DNS_ERR_NONE);
        REQUIRE(response.get_failure() == "");
        REQUIRE(response.get_results().size() == 1);
        REQUIRE(response.get_results()[0] == "server-nexa.polito.it");
        REQUIRE(response.get_rtt() > 0.0);
        REQUIRE(response.get_ttl() > 0);
        ight_break_loop();
    });
    ight_loop();

    auto r3 = reso.request("AAAA", "ooni.torproject.org", [&](
                           ight::DNSResponse&& response) {
        REQUIRE(response.get_query_name() == "ooni.torproject.org");
        REQUIRE(response.get_query_type() == "AAAA");
        REQUIRE(response.get_query_class() == "IN");
        REQUIRE(response.get_reply_authoritative() == "unknown");
        REQUIRE(response.get_resolver()[0] == "<default>");
        REQUIRE(response.get_resolver()[1] == "53");
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

    auto r4 = reso.request("REVERSE_AAAA", "2001:858:2:2:aabb:0:563b:1e28",
                           [&](ight::DNSResponse&& response) {
        REQUIRE(response.get_query_name() == "2001:858:2:2:aabb:0:563b:1e28");
        REQUIRE(response.get_query_type() == "PTR");
        REQUIRE(response.get_query_class() == "IN");
        REQUIRE(response.get_reply_authoritative() == "unknown");
        REQUIRE(response.get_resolver()[0] == "<default>");
        REQUIRE(response.get_resolver()[1] == "53");
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

    auto d = IghtDelayedCall(10.0, [&](void) {
        failed = true;
        ight_break_loop();
    });

    auto reso = ight::DNSResolver("8.8.4.4");

    auto r1 = reso.request("A", "www.neubot.org", [&](
                           ight::DNSResponse&& response) {
        REQUIRE(response.get_query_name() == "www.neubot.org");
        REQUIRE(response.get_query_type() == "A");
        REQUIRE(response.get_query_class() == "IN");
        REQUIRE(response.get_reply_authoritative() == "unknown");
        REQUIRE(response.get_resolver()[0] == "8.8.4.4");
        REQUIRE(response.get_resolver()[1] == "53");
        REQUIRE(response.get_evdns_status() == DNS_ERR_NONE);
        REQUIRE(response.get_failure() == "");
        REQUIRE(response.get_results().size() == 1);
        REQUIRE(response.get_results()[0] == "130.192.16.172");
        REQUIRE(response.get_rtt() > 0.0);
        REQUIRE(response.get_ttl() > 0);
        ight_break_loop();
    });
    ight_loop();

    auto r2 = reso.request("REVERSE_A", "130.192.16.172", [&](
                           ight::DNSResponse&& response) {
        REQUIRE(response.get_query_name() == "130.192.16.172");
        REQUIRE(response.get_query_type() == "PTR");
        REQUIRE(response.get_query_class() == "IN");
        REQUIRE(response.get_reply_authoritative() == "unknown");
        REQUIRE(response.get_resolver()[0] == "8.8.4.4");
        REQUIRE(response.get_resolver()[1] == "53");
        REQUIRE(response.get_evdns_status() == DNS_ERR_NONE);
        REQUIRE(response.get_failure() == "");
        REQUIRE(response.get_results().size() == 1);
        REQUIRE(response.get_results()[0] == "server-nexa.polito.it");
        REQUIRE(response.get_rtt() > 0.0);
        REQUIRE(response.get_ttl() > 0);
        ight_break_loop();
    });
    ight_loop();

    auto r3 = reso.request("AAAA", "ooni.torproject.org", [&](
                           ight::DNSResponse&& response) {
        REQUIRE(response.get_query_name() == "ooni.torproject.org");
        REQUIRE(response.get_query_type() == "AAAA");
        REQUIRE(response.get_query_class() == "IN");
        REQUIRE(response.get_reply_authoritative() == "unknown");
        REQUIRE(response.get_resolver()[0] == "8.8.4.4");
        REQUIRE(response.get_resolver()[1] == "53");
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

    auto r4 = reso.request("REVERSE_AAAA", "2001:858:2:2:aabb:0:563b:1e28",
                           [&](ight::DNSResponse&& response) {
        REQUIRE(response.get_query_name() == "2001:858:2:2:aabb:0:563b:1e28");
        REQUIRE(response.get_query_type() == "PTR");
        REQUIRE(response.get_query_class() == "IN");
        REQUIRE(response.get_reply_authoritative() == "unknown");
        REQUIRE(response.get_resolver()[0] == "8.8.4.4");
        REQUIRE(response.get_resolver()[1] == "53");
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

TEST_CASE("If the resolver dies, the requests are aborted") {

    // I need to remember to never run a DNS on that machine :^)
    auto reso = new ight::DNSResolver("130.192.91.231");
    auto r1 = reso->request("A", "www.neubot.org", [&](
                            ight::DNSResponse&& response) {
        REQUIRE(response.get_query_name() == "www.neubot.org");
        REQUIRE(response.get_query_type() == "A");
        REQUIRE(response.get_query_class() == "IN");
        REQUIRE(response.get_reply_authoritative() == "unknown");
        REQUIRE(response.get_resolver()[0] == "130.192.91.231");
        REQUIRE(response.get_resolver()[1] == "53");
        REQUIRE(response.get_results().size() == 0);
        REQUIRE(response.get_evdns_status() == DNS_ERR_SHUTDOWN);
        REQUIRE(response.get_failure() == "unknown failure 68");
        REQUIRE(response.get_ttl() == 0);
        REQUIRE(response.get_rtt() == 0.0);
        ight_break_loop();
    });

    auto d1 = IghtDelayedCall(0.1, [&](void) {
        delete reso;  // Destroy the resolver and see what happens..
                      // in theory the callback above should be called
    });

    auto failed = false;
    auto d2 = IghtDelayedCall(1.0, [&](void) {
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
    auto reso = ight::DNSResolver("130.192.91.231", 1);
    auto r1 = reso.request("A", "www.neubot.org", [&](
                           ight::DNSResponse&& response) {
        REQUIRE(response.get_query_name() == "www.neubot.org");
        REQUIRE(response.get_query_type() == "A");
        REQUIRE(response.get_query_class() == "IN");
        REQUIRE(response.get_reply_authoritative() == "unknown");
        REQUIRE(response.get_resolver()[0] == "130.192.91.231");
        REQUIRE(response.get_resolver()[1] == "53");
        REQUIRE(response.get_results().size() == 0);
        REQUIRE(response.get_evdns_status() == DNS_ERR_TIMEOUT);
        REQUIRE(response.get_failure() == "deferred_timeout_error");
        REQUIRE(response.get_ttl() == 0);
        REQUIRE(response.get_rtt() == 0.0);
        ight_break_loop();
    });

    auto failed = false;
    auto d = IghtDelayedCall(10.0, [&](void) {
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

    auto reso = ight::DNSResolver("8.8.8.8", 1);

    // Step #1: estimate the average RTT

    auto total = 0.0;
    auto count = 0;
    for (auto i = 0; i < 16; ++i) {
        auto r = reso.request("A", "www.neubot.org", [&](
                              ight::DNSResponse&& response) {
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
        auto r = new ight::DNSRequest("A", "www.neubot.org", [&](
                                      ight::DNSResponse&& response) {
            auto status_ok = (response.get_evdns_status() == DNS_ERR_CANCEL
                    || response.get_evdns_status() == DNS_ERR_NONE);
            REQUIRE(status_ok);
            // Ignoring all the other fields here
            ight_warn("- break_loop");
            ight_break_loop();
        }, reso.get_evdns_base());
        auto d = IghtDelayedCall(avgrtt, [&](void) {
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
    auto reso = ight::DNSResolver("127.0.0.1:5353", 2);
    auto r = reso.request("A", "www.neubot.org", [&](
                          ight::DNSResponse response) {
        REQUIRE(response.get_results().size() == 0);
        REQUIRE(response.get_evdns_status() == DNS_ERR_TIMEOUT);
        // Assuming all the other fields are OK
        ight_break_loop();
    });
    ight_loop();
}
*/
