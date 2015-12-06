// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <measurement_kit/dns.hpp>
#include <measurement_kit/common.hpp>

#include "src/common/delayed_call.hpp"
#include "src/common/utils.hpp"
#include "src/dns/query.hpp"
#include "src/common/check_connectivity.hpp"

using namespace mk;
using namespace mk::dns;

//
// Resolver unit tests.
//

// Now testing: cleanup()

TEST_CASE("Resolver: cleanup works correctly when we have allocated") {
    auto libs = Libs();

    auto called = 0;
    libs.evdns_base_free = [&](evdns_base *p, int f) {
        ::evdns_base_free(p, f);
        called++;
    };

    {
        // Note: call .get_evdns_base() to trigger lazy allocation
        Resolver(Settings(), Logger::global(), &libs).get_evdns_base();
    }

    REQUIRE(called == 1);
}

TEST_CASE("Resolver: cleanup works correctly when we have not allocated") {
    auto libs = Libs();

    auto called = 0;
    libs.evdns_base_free = [&](evdns_base *p, int f) {
        ::evdns_base_free(p, f);
        called++;
    };

    { Resolver(); }

    REQUIRE(called == 0);
}

// Now testing Resolver(...)

TEST_CASE("Resolver: ensure that the constructor does not allocate") {
    auto libs = Libs();

    libs.evdns_base_new = [](event_base *, int) { return (evdns_base *)NULL; };

    //
    // Basically: if we go through the end we have not allocated because
    // if we try to allocate we fail and the code will raise
    //

    // Resolver();  // How to do this?
    Resolver(Settings(), Logger::global(), &libs);
}

TEST_CASE("Resolver: evdns_base_new failure is correctly handled") {
    auto libs = Libs();

    libs.evdns_base_new = [](event_base *, int) { return (evdns_base *)NULL; };

    // Note: call .get_evdns_base() to trigger lazy allocation

    // Handle the branch where nameserver is set
    REQUIRE_THROWS(Resolver({{"nameserver", "8.8.8.8"}}, Logger::global(),
                            &libs).get_evdns_base());

    // Handle the branch using the default nameserver
    REQUIRE_THROWS(
        Resolver(Settings(), Logger::global(), &libs).get_evdns_base());
}

TEST_CASE(
    "Resolver: evdns_base_nameserver_ip_add failure is correctly handled") {
    auto libs = Libs();

    libs.evdns_base_nameserver_ip_add =
        [](evdns_base *, const char *) { return -1; };

    // Also make sure that the destructor is called
    auto called = false;
    libs.evdns_base_free = [&](evdns_base *p, int f) {
        ::evdns_base_free(p, f);
        called = true;
    };

    // Note: call .get_evdns_base() to trigger lazy allocation
    REQUIRE_THROWS(Resolver({{"nameserver", "8.8.8.8"}}, Logger::global(),
                            &libs).get_evdns_base());

    REQUIRE(called);
}

TEST_CASE("Resolver: evdns_base_set_option failure is correctly handled") {
    auto libs = Libs();

    // Also make sure that the destructor is called
    auto called = 0;
    libs.evdns_base_free = [&](evdns_base *p, int f) {
        ::evdns_base_free(p, f);
        called += 1;
    };

    // Note: call .get_evdns_base() to trigger lazy allocation

    libs.evdns_base_set_option =
        [](evdns_base *, const char *opt, const char *) {
            if (strcmp(opt, "attempts") == 0) {
                return -1;
            }
            return 0;
        };
    REQUIRE_THROWS(Resolver(
                       {
                        {"attempts", "1"},
                       },
                       Logger::global(), &libs).get_evdns_base());

    libs.evdns_base_set_option =
        [](evdns_base *, const char *opt, const char *) {
            if (strcmp(opt, "timeout") == 0) {
                return -1;
            }
            return 0;
        };
    REQUIRE_THROWS(Resolver(
                       {
                        {"timeout", "1.0"},
                       },
                       Logger::global(), &libs).get_evdns_base());

    libs.evdns_base_set_option =
        [](evdns_base *, const char *opt, const char *) {
            if (strcmp(opt, "randomize-case") == 0) {
                return -1;
            }
            return 0;
        };
    // Make sure that randomize-case is called in both true and false cases
    REQUIRE_THROWS(Resolver(
                       {
                        {"randomize_case", "1"},
                       },
                       Logger::global(), &libs).get_evdns_base());
    REQUIRE_THROWS(Resolver(
                       {
                        {"randomize_case", "0"},
                       },
                       Logger::global(), &libs).get_evdns_base());

    REQUIRE(called == 4); // twice for randomize-case
}

TEST_CASE("Resolver::get_evdns_base() is idempotent") {
    Resolver reso;
    REQUIRE(reso.get_evdns_base() == reso.get_evdns_base());
}

TEST_CASE("We can override the default timeout") {

    // I need to remember to never run a DNS on that machine :^)
    Resolver reso(Settings{{"nameserver", "130.192.91.231"},
                           {"attempts", "1"},
                           {"timeout", "0.5"}});

    auto ticks = mk::time_now();
    reso.query("IN", "A", "www.neubot.org", [&](Error e, Response response) {
        REQUIRE(e == TimeoutError());
        REQUIRE(response.get_reply_authoritative() == "unknown");
        REQUIRE(response.get_results().size() == 0);
        REQUIRE(response.get_evdns_status() == DNS_ERR_TIMEOUT);
        REQUIRE(response.get_ttl() == 0);
        REQUIRE(response.get_rtt() == 0.0);

        auto elapsed = mk::time_now() - ticks;
        // This interval is wide so the unit tests does not fail when
        // we run it using valgrind on slow machines
        REQUIRE(elapsed > 0.3);
        REQUIRE(elapsed < 0.7);

        mk::break_loop();
    });
    mk::loop();
}

TEST_CASE("We can override the default number of tries") {

    // I need to remember to never run a DNS on that machine :^)
    Resolver reso(Settings{
        {"nameserver", "130.192.91.231"}, {"attempts", "2"}, {"timeout", "0.5"},
    });

    auto ticks = mk::time_now();
    reso.query("IN", "A", "www.neubot.org", [&](Error e, Response response) {
        REQUIRE(e == TimeoutError());
        REQUIRE(response.get_reply_authoritative() == "unknown");
        REQUIRE(response.get_results().size() == 0);
        REQUIRE(response.get_evdns_status() == DNS_ERR_TIMEOUT);
        REQUIRE(response.get_ttl() == 0);
        REQUIRE(response.get_rtt() == 0.0);

        auto elapsed = mk::time_now() - ticks;
        REQUIRE(elapsed > 0.8);
        REQUIRE(elapsed < 1.2);

        mk::break_loop();
    });
    mk::loop();
}

//
// Intregration (or regress?) tests for Resolver.
//
// They generally need connectivity and are automatically skipped if
// we are not connected to the 'Net.
//

TEST_CASE("The default custom resolver works as expected") {

    if (CheckConnectivity::is_down()) {
        return;
    }

    auto failed = false;

    DelayedCall d(10.0, [&](void) {
        failed = true;
        mk::break_loop();
    });

    Resolver reso;

    reso.query("IN", "A", "www.neubot.org", [&](Error e, Response response) {
        REQUIRE(!e);
        REQUIRE(response.get_reply_authoritative() == "unknown");
        REQUIRE(response.get_evdns_status() == DNS_ERR_NONE);
        REQUIRE(response.get_results().size() == 1);
        REQUIRE(response.get_results()[0] == "130.192.16.172");
        REQUIRE(response.get_rtt() > 0.0);
        REQUIRE(response.get_ttl() > 0);
        mk::break_loop();
    });
    mk::loop();

    reso.query(
        "IN", "REVERSE_A", "130.192.16.172", [&](Error e, Response response) {
            REQUIRE(!e);
            REQUIRE(response.get_reply_authoritative() == "unknown");
            REQUIRE(response.get_evdns_status() == DNS_ERR_NONE);
            REQUIRE(response.get_results().size() == 1);
            REQUIRE(response.get_results()[0] == "server-nexa.polito.it");
            REQUIRE(response.get_rtt() > 0.0);
            REQUIRE(response.get_ttl() > 0);
            mk::break_loop();
        });
    mk::loop();

    reso.query("IN", "PTR", "172.16.192.130.in-addr.arpa.",
               [&](Error e, Response response) {
                   REQUIRE(!e);
                   REQUIRE(response.get_reply_authoritative() == "unknown");
                   REQUIRE(response.get_evdns_status() == DNS_ERR_NONE);
                   REQUIRE(response.get_results().size() == 1);
                   REQUIRE(response.get_results()[0] ==
                           "server-nexa.polito.it");
                   REQUIRE(response.get_rtt() > 0.0);
                   REQUIRE(response.get_ttl() > 0);
                   mk::break_loop();
               });
    mk::loop();

    reso.query("IN", "AAAA", "ooni.torproject.org",
               [&](Error e, Response response) {
                   REQUIRE(!e);
                   REQUIRE(response.get_reply_authoritative() == "unknown");
                   REQUIRE(response.get_evdns_status() == DNS_ERR_NONE);
                   REQUIRE(response.get_results().size() > 0);
                   REQUIRE(response.get_rtt() > 0.0);
                   REQUIRE(response.get_ttl() > 0);
                   auto found = false;
                   for (auto address : response.get_results()) {
                       if (address == "2001:41b8:202:deb:213:21ff:fe20:1426") {
                           found = true;
                       }
                   }
                   REQUIRE(found);
                   mk::break_loop();
               });
    mk::loop();

    reso.query("IN", "REVERSE_AAAA", "2001:41b8:202:deb:213:21ff:fe20:1426",
               [&](Error e, Response response) {
                   REQUIRE(!e);
                   REQUIRE(response.get_reply_authoritative() == "unknown");
                   REQUIRE(response.get_evdns_status() == DNS_ERR_NONE);
                   REQUIRE(response.get_results().size() == 1);
                   REQUIRE(response.get_results()[0] ==
                           "listera.torproject.org");
                   REQUIRE(response.get_rtt() > 0.0);
                   REQUIRE(response.get_ttl() > 0);
                   mk::break_loop();
               });
    mk::loop();

    reso.query("IN", "PTR", "6.2.4.1.0.2.e.f.f.f.1.2.3.1.2.0.b.e.d.0.2.0.2.0.8."
                            "b.1.4.1.0.0.2.ip6.arpa.",
               [&](Error e, Response response) {
                   REQUIRE(!e);
                   REQUIRE(response.get_reply_authoritative() == "unknown");
                   REQUIRE(response.get_evdns_status() == DNS_ERR_NONE);
                   REQUIRE(response.get_results().size() == 1);
                   REQUIRE(response.get_results()[0] ==
                           "listera.torproject.org");
                   REQUIRE(response.get_rtt() > 0.0);
                   REQUIRE(response.get_ttl() > 0);
                   mk::break_loop();
               });
    mk::loop();

    REQUIRE(!failed);
}

TEST_CASE("A specific custom resolver works as expected") {

    if (CheckConnectivity::is_down()) {
        return;
    }

    auto failed = false;

    DelayedCall d(10.0, [&](void) {
        failed = true;
        mk::break_loop();
    });

    Resolver reso(Settings({
        {"nameserver", "8.8.4.4"},
    }));

    reso.query("IN", "A", "www.neubot.org", [&](Error e, Response response) {
        REQUIRE(!e);
        REQUIRE(response.get_reply_authoritative() == "unknown");
        REQUIRE(response.get_evdns_status() == DNS_ERR_NONE);
        REQUIRE(response.get_results().size() == 1);
        REQUIRE(response.get_results()[0] == "130.192.16.172");
        REQUIRE(response.get_rtt() > 0.0);
        REQUIRE(response.get_ttl() > 0);
        mk::break_loop();
    });
    mk::loop();

    reso.query(
        "IN", "REVERSE_A", "130.192.16.172", [&](Error e, Response response) {
            REQUIRE(!e);
            REQUIRE(response.get_reply_authoritative() == "unknown");
            REQUIRE(response.get_evdns_status() == DNS_ERR_NONE);
            REQUIRE(response.get_results().size() == 1);
            REQUIRE(response.get_results()[0] == "server-nexa.polito.it");
            REQUIRE(response.get_rtt() > 0.0);
            REQUIRE(response.get_ttl() > 0);
            mk::break_loop();
        });
    mk::loop();

    reso.query("IN", "PTR", "172.16.192.130.in-addr.arpa.",
               [&](Error e, Response response) {
                   REQUIRE(!e);
                   REQUIRE(response.get_reply_authoritative() == "unknown");
                   REQUIRE(response.get_evdns_status() == DNS_ERR_NONE);
                   REQUIRE(response.get_results().size() == 1);
                   REQUIRE(response.get_results()[0] ==
                           "server-nexa.polito.it");
                   REQUIRE(response.get_rtt() > 0.0);
                   REQUIRE(response.get_ttl() > 0);
                   mk::break_loop();
               });
    mk::loop();

    reso.query("IN", "AAAA", "ooni.torproject.org",
               [&](Error e, Response response) {
                   REQUIRE(!e);
                   REQUIRE(response.get_reply_authoritative() == "unknown");
                   REQUIRE(response.get_evdns_status() == DNS_ERR_NONE);
                   REQUIRE(response.get_results().size() > 0);
                   REQUIRE(response.get_rtt() > 0.0);
                   REQUIRE(response.get_ttl() > 0);
                   auto found = false;
                   for (auto address : response.get_results()) {
                       if (address == "2001:41b8:202:deb:213:21ff:fe20:1426") {
                           found = true;
                       }
                   }
                   REQUIRE(found);
                   mk::break_loop();
               });
    mk::loop();

    reso.query("IN", "REVERSE_AAAA", "2001:41b8:202:deb:213:21ff:fe20:1426",
               [&](Error e, Response response) {
                   REQUIRE(!e);
                   REQUIRE(response.get_reply_authoritative() == "unknown");
                   REQUIRE(response.get_evdns_status() == DNS_ERR_NONE);
                   REQUIRE(response.get_results().size() == 1);
                   REQUIRE(response.get_results()[0] ==
                           "listera.torproject.org");
                   REQUIRE(response.get_rtt() > 0.0);
                   REQUIRE(response.get_ttl() > 0);
                   mk::break_loop();
               });
    mk::loop();

    reso.query("IN", "PTR", "6.2.4.1.0.2.e.f.f.f.1.2.3.1.2.0.b.e.d.0.2.0.2.0.8."
                            "b.1.4.1.0.0.2.ip6.arpa.",
               [&](Error e, Response response) {
                   REQUIRE(!e);
                   REQUIRE(response.get_reply_authoritative() == "unknown");
                   REQUIRE(response.get_evdns_status() == DNS_ERR_NONE);
                   REQUIRE(response.get_results().size() == 1);
                   REQUIRE(response.get_results()[0] ==
                           "listera.torproject.org");
                   REQUIRE(response.get_rtt() > 0.0);
                   REQUIRE(response.get_ttl() > 0);
                   mk::break_loop();
               });
    mk::loop();

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

    reso->query("IN", "A", "www.neubot.org", [&](Error e, Response response) {
        REQUIRE(e == ShutdownError());
        REQUIRE(response.get_reply_authoritative() == "unknown");
        REQUIRE(response.get_results().size() == 0);
        REQUIRE(response.get_evdns_status() == DNS_ERR_SHUTDOWN);
        REQUIRE(response.get_ttl() == 0);
        REQUIRE(response.get_rtt() == 0.0);
        mk::break_loop();
    });

    DelayedCall d1(0.1, [&](void) {
        delete reso; // Destroy the resolver and see what happens..
                     // in theory the request callback *should* be called
    });

    auto failed = false;
    DelayedCall d2(1.0, [&](void) {
        // This *should not* be called, since the request callback
        // shold be called before than this one.
        failed = true;
        mk::break_loop();
    });

    mk::loop();

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
    // if (CheckConnectivity::is_down()) {
    //    return;
    //}
    //

    // I need to remember to never run a DNS on that machine :^)
    Resolver reso(Settings{
        {"nameserver", "130.192.91.231"}, {"attempts", "1"},
    });
    reso.query("IN", "A", "www.neubot.org", [&](Error e, Response response) {
        REQUIRE(e == TimeoutError());
        REQUIRE(response.get_reply_authoritative() == "unknown");
        REQUIRE(response.get_results().size() == 0);
        REQUIRE(response.get_evdns_status() == DNS_ERR_TIMEOUT);
        REQUIRE(response.get_ttl() == 0);
        REQUIRE(response.get_rtt() == 0.0);
        mk::break_loop();
    });

    auto failed = false;
    DelayedCall d(10.0, [&](void) {
        failed = true;
        mk::break_loop();
    });

    mk::loop();

    REQUIRE(!failed);
}

TEST_CASE("It is safe to cancel requests in flight") {

    if (CheckConnectivity::is_down()) {
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

    Resolver reso(Settings{
        {"nameserver", "8.8.8.8"}, {"attempts", "1"},
    });

    // Step #1: estimate the average RTT

    auto total = 0.0;
    auto count = 0;
    for (auto i = 0; i < 16; ++i) {
        reso.query("IN", "A", "www.neubot.org",
                   [&](Error e, Response response) {
                       if (!e) {
                           total += response.get_rtt();
                           count += 1;
                       }
                       mk::break_loop();
                   });
        mk::loop();
    }
    // We need at lest 8 good responses to compute the average
    // We do not require 16 to tolerate a few losses
    REQUIRE(count > 8);
    auto avgrtt = total / count;

    // Step #2: attempt to unschedule responses when they are due

    // for (;;) {  // only try this at home
    for (auto i = 0; i < 16; ++i) {
        auto r = new Query("IN", "A", "www.neubot.org", [&](Error e, Response) {
            auto status_ok = (e == CancelError() || e == NoError());
            REQUIRE(status_ok);
            // Ignoring all the other fields here
            mk::warn("- break_loop");
            mk::break_loop();
        }, Logger::global(), reso.get_evdns_base());
        DelayedCall d(avgrtt, [&](void) {
            mk::warn("- cancel");
            r->cancel();
            mk::break_loop();
        });
        mk::loop();
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
    reso.query("IN", "A", "www.neubot.org", [&](Error e, Response response) {
        REQUIRE(response.get_results().size() == 0);
        REQUIRE(e == TimeoutError());
        // Assuming all the other fields are OK
        mk::break_loop();
    });
    mk::loop();
}
*/
