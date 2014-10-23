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

TEST_CASE("The system resolver works as expected") {

    auto d = IghtDelayedCall(10.0, [](void) {
        throw std::runtime_error("Test failed");
    });

    auto r1 = ight::DNSRequest("A", "www.neubot.org", [&](
      ight::DNSResponse&& response) {
        REQUIRE(response.results.size() == 1);
        REQUIRE(response.results[0] == "130.192.16.172");
        ight_break_loop();
    });
    ight_loop();

    auto r2 = ight::DNSRequest("REVERSE_A", "130.192.16.172", [&](
      ight::DNSResponse&& response) {
        REQUIRE(response.results.size() == 1);
        REQUIRE(response.results[0] == "server-nexa.polito.it");
        ight_break_loop();
    });
    ight_loop();

    auto r3 = ight::DNSRequest("AAAA", "ooni.torproject.org", [&](
                               ight::DNSResponse&& response) {
        REQUIRE(response.results.size() > 0);
        auto found = false;
        for (auto address : response.results) {
            if (address == "2001:858:2:2:aabb:0:563b:1e28") {
                found = true;
            }
        }
        REQUIRE(found);
        ight_break_loop();
    });
    ight_loop();

    auto r4 = ight::DNSRequest("REVERSE_AAAA",
                         "2001:858:2:2:aabb:0:563b:1e28",
                         [&](ight::DNSResponse&& response) {
        REQUIRE(response.results.size() == 1);
        REQUIRE(response.results[0] == "nova.torproject.org");
        ight_break_loop();
    });
    ight_loop();
}

TEST_CASE("The default custom resolver works as expected") {

    auto d = IghtDelayedCall(10.0, [](void) {
        throw std::runtime_error("Test failed");
    });

    auto reso = ight::DNSResolver();

    auto r1 = ight::DNSRequest("A", "www.neubot.org", [&](
      ight::DNSResponse&& response) {
        REQUIRE(response.results.size() == 1);
        REQUIRE(response.results[0] == "130.192.16.172");
        ight_break_loop();
    }, reso);
    ight_loop();

    auto r2 = ight::DNSRequest("REVERSE_A", "130.192.16.172", [&](
      ight::DNSResponse&& response) {
        REQUIRE(response.results.size() == 1);
        REQUIRE(response.results[0] == "server-nexa.polito.it");
        ight_break_loop();
    }, reso);
    ight_loop();

    auto r3 = ight::DNSRequest("AAAA", "ooni.torproject.org", [&](
                               ight::DNSResponse&& response) {
        REQUIRE(response.results.size() > 0);
        auto found = false;
        for (auto address : response.results) {
            if (address == "2001:858:2:2:aabb:0:563b:1e28") {
                found = true;
            }
        }
        REQUIRE(found);
        ight_break_loop();
    }, reso);
    ight_loop();

    auto r4 = ight::DNSRequest("REVERSE_AAAA",
                         "2001:858:2:2:aabb:0:563b:1e28",
                         [&](ight::DNSResponse&& response) {
        REQUIRE(response.results.size() == 1);
        REQUIRE(response.results[0] == "nova.torproject.org");
        ight_break_loop();
    }, reso);
    ight_loop();
}

TEST_CASE("A specific custom resolver works as expected") {

    auto d = IghtDelayedCall(10.0, [](void) {
        throw std::runtime_error("Test failed");
    });

    auto reso = ight::DNSResolver("8.8.4.4");

    auto r1 = ight::DNSRequest("A", "www.neubot.org", [&](
      ight::DNSResponse&& response) {
        REQUIRE(response.results.size() == 1);
        REQUIRE(response.results[0] == "130.192.16.172");
        ight_break_loop();
    }, reso);
    ight_loop();

    auto r2 = ight::DNSRequest("REVERSE_A", "130.192.16.172", [&](
      ight::DNSResponse&& response) {
        REQUIRE(response.results.size() == 1);
        REQUIRE(response.results[0] == "server-nexa.polito.it");
        ight_break_loop();
    }, reso);
    ight_loop();

    auto r3 = ight::DNSRequest("AAAA", "ooni.torproject.org", [&](
                               ight::DNSResponse&& response) {
        REQUIRE(response.results.size() > 0);
        auto found = false;
        for (auto address : response.results) {
            if (address == "2001:858:2:2:aabb:0:563b:1e28") {
                found = true;
            }
        }
        REQUIRE(found);
        ight_break_loop();
    }, reso);
    ight_loop();

    auto r4 = ight::DNSRequest("REVERSE_AAAA",
                         "2001:858:2:2:aabb:0:563b:1e28",
                         [&](ight::DNSResponse&& response) {
        REQUIRE(response.results.size() == 1);
        REQUIRE(response.results[0] == "nova.torproject.org");
        ight_break_loop();
    }, reso);
    ight_loop();
}

TEST_CASE("A request to a nonexistent server times out") {

    auto reso = ight::DNSResolver("130.192.91.231", "1");
    auto r1 = ight::DNSRequest("A", "www.neubot.org", [&](
                               ight::DNSResponse&& response) {
        REQUIRE(response.results.size() == 0);
        REQUIRE(response.code == DNS_ERR_TIMEOUT);
        ight_break_loop();
    }, reso);

    auto d = IghtDelayedCall(10.0, [](void) {
        throw std::runtime_error("Test failed");
    });

    ight_loop();
}

TEST_CASE("It is safe to pass a dying DNSResolver to DNSRequest") {

    auto r1 = ight::DNSRequest("A", "www.neubot.org", [&](
                               ight::DNSResponse&& response) {
        REQUIRE(response.results.size() == 0);
        REQUIRE(response.code == DNS_ERR_SHUTDOWN);
        ight_break_loop();
    }, ight::DNSResolver("130.192.91.231"));

    auto d = IghtDelayedCall(1.0, [](void) {
        throw std::runtime_error("Test failed");
    });

    ight_loop();
}

TEST_CASE("It is safe to forget about pending requests") {

    {
        auto r1 = ight::DNSRequest("A", "www.neubot.org", [&](
                                   ight::DNSResponse&& /*response*/) {
            throw std::runtime_error("Should not happen");
        });

    }  // This should kill r1

    auto d = IghtDelayedCall(5.0, [](void) {
        ight_break_loop();  // We should receive a response earlier than this
    });

    ight_loop();
}

TEST_CASE("It is safe to cancel requests in flight") {

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
        auto r = ight::DNSRequest("A", "www.neubot.org", [&](
                                  ight::DNSResponse&& response) {
            total += response.rtt;
            count += 1;
            ight_break_loop();
        }, reso);
        ight_loop();
    }
    auto avgrtt = total / count;

    // Step #2: attempt to unschedule responses when they are due

    //for (;;) {  // only try this at home
    for (auto i = 0; i < 16; ++i) {
        auto r = new ight::DNSRequest("A", "www.neubot.org", [&](
                                      ight::DNSResponse&& /*response*/) {
            ight_warn("- break_loop");
            ight_break_loop();
        }, reso);
        auto d = IghtDelayedCall(avgrtt, [&](void) {
            ight_warn("- cancel");
            delete r;
            ight_break_loop();
        });
        ight_loop();
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
    auto r = ight::DNSRequest("A", "www.neubot.org", [&](ight::DNSResponse) {
        throw std::runtime_error("This should not happen");
    }, reso);
    ight_loop();
}
*/
