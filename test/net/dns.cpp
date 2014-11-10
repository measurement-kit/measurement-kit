/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

//
// Regression tests for `net/dns.hpp`
//

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include "net/dns.hpp"

#include "common/log.h"

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

TEST_CASE("The empty DNSResponse has sensible fields") {
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

TEST_CASE("DNSRequest raises if the query is unsupported") {
    REQUIRE_THROWS(ight::DNSRequest("PTR", "www.neubot.org",
                   [&](ight::DNSResponse&& /*response*/) {
        // nothing
    }));
}

TEST_CASE("The system resolver works as expected") {

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

TEST_CASE("Cancel is idempotent") {

    auto r1 = ight::DNSRequest("A", "www.neubot.org", [&](
                               ight::DNSResponse&& /*response*/) {
        // nothing
    });

    r1.cancel();
    r1.cancel();
    r1.cancel();

    // Here we only want to see that multiple cancel()s followed
    // by the object being destroyed cause no harm
}

TEST_CASE("A request to a nonexistent server times out") {

    //
    // Note: this test also makes sense when the network is down, because,
    // when the network is down, the DNS server is, in a sense, nonexistent,
    // i.e., the request is aborted due to a timeout.
    //

    auto reso = ight::DNSResolver("130.192.91.231", "1");
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

TEST_CASE("If the resolver dies, the requests are aborted") {

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
        delete reso;  // Destroy the resolver and see what happens
    });

    auto failed = false;
    auto d2 = IghtDelayedCall(1.0, [&](void) {
        failed = true;
        ight_break_loop();
    });

    ight_loop();

    REQUIRE(!failed);
}

TEST_CASE("It is safe to forget about pending requests") {

    //
    // Note: this test makes sense also when the network is down, because
    // the internal DNSRequestImpl is killed by a timeout rather than by
    // the receipt of a response, as happens when the network is up.
    //

    auto failed = false;

    {
        auto r1 = ight::DNSRequest("A", "www.neubot.org", [&](
                                   ight::DNSResponse&& /*response*/) {
            //
            // Should not happen because when r1 dies the callback
            // must not be invoked by the underlying DNSRequestImpl
            //
            failed = true;
            ight_break_loop();
        });

    }  // This should kill r1

    auto d = IghtDelayedCall(5.0, [](void) {
        //
        // After 5 seconds we should have received a response for
        // www.neubot.org and the internal DNSRequestImpl should be
        // already dead (yes, this is a race-condition-based test
        // so we use 5 seconds to be on the safe side).
        //
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

    auto reso = ight::DNSResolver("8.8.8.8", "1");

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
    auto reso = ight::DNSResolver("127.0.0.1:5353", "2");
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
