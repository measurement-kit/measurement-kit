// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/libmeasurement_kit/ext/catch.hpp"

#include "src/libmeasurement_kit/dns/ping.hpp"

#include "src/libmeasurement_kit/dns/query.hpp"

#include <iostream>

using namespace mk;

#ifdef ENABLE_INTEGRATION_TESTS

TEST_CASE("The dns::ping() template works") {
    SharedPtr<Reactor> reactor = Reactor::make();
    double now = time_now();
    Settings settings{
          {"dns/nameserver", "8.8.8.8:53"},
          {"dns/timeout", 0.5},
          {"dns/retries", 1},
          {"dns/engine", "libevent"},
    };
    reactor->run_with_initial_event([&]() {
        dns::ping_nameserver("IN", "A", "www.google.com", 1.0,
                             Maybe<double>{10.0}, settings, reactor,
                             Logger::global(),
                             [](Error err, SharedPtr<dns::Message> message) {
                                 std::cout << err;
                                 if (message) {
                                     std::cout << " " << message->error_code;
                                     for (auto &a : message->answers) {
                                         std::cout << " " << a.ipv4;
                                     }
                                 }
                                 std::cout << "\n";
                             },
                             [&](Error err) {
                                 REQUIRE(err == NoError());
                                 reactor->stop();
                             });
    });
    double delta = time_now() - now;
    REQUIRE(delta > 9.0);
    REQUIRE(delta < 11.0);
}

#endif
