// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include "src/common/check_connectivity.hpp"
#include <atomic>
#include <chrono>
#include <measurement_kit/common.hpp>
#include <measurement_kit/dns.hpp>
#include <measurement_kit/http.hpp>
#include <measurement_kit/net.hpp>
#include <thread>

using namespace mk;

TEST_CASE("The detached reactor works") {
    if (CheckConnectivity::is_down()) {
        return;
    }

    // Here we just do some concurrent networking in the background thread
    // coupled with some REQUIRE:s to show this can be done.

    set_verbose(1);
    Var<Reactor> reactor = Reactor::global_detached();
    std::atomic<int> count{5};

    dns::query("IN", "A", "nexa.polito.it",
               [&count](Error err, dns::Message msg) {
                   REQUIRE(!err);
                   REQUIRE(msg.answers.size() == 1);
                   REQUIRE(msg.answers[0].ipv4 == "130.192.16.172");
                   --count;
               },
               {}, reactor);

    dns::query("IN", "A", "img.polito.it",
               [&count](Error err, dns::Message msg) {
                   REQUIRE(!err);
                   REQUIRE(msg.answers.size() == 1);
                   REQUIRE(msg.answers[0].ipv4 == "130.192.225.140");
                   --count;
               },
               {}, reactor);

    dns::query("IN", "A", "www.polito.it",
               [&count](Error err, dns::Message msg) {
                   REQUIRE(!err);
                   REQUIRE(msg.answers.size() == 1);
                   REQUIRE(msg.answers[0].ipv4 == "130.192.181.193");
                   --count;
               },
               {}, reactor);

    http::get("http://google.com/robots.txt",
              [&count](Error err, http::Response r) {
                  REQUIRE(!err);
                  REQUIRE(r.status_code == 301);
                  --count;
              },
              {}, "", {}, Logger::global(), reactor);

    net::connect("ooni.torproject.org", 80,
                 [&count](Error err, Var<net::Transport> txp) {
                     REQUIRE(!err);
                     txp->close([&count]() { --count; });
                 },
                 {}, Logger::global(), reactor);

    while (count > 0) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Break the loop to terminate background thread and wait one more
    // second such that the thread actually terminates. Strictly speaking,
    // this code would not be needed, but not terminating the background
    // thread properly makes Valgrind complain.
    reactor->break_loop();
    std::this_thread::sleep_for(std::chrono::seconds(1));
}
