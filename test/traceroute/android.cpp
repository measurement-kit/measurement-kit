// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#ifdef ENABLE_INTEGRATION_TESTS

/// Tests Android traceroute prober

// This is meant to run on Android but can run on all Linux systems
#if (defined __linux__ && defined ENABLE_TRACEROUTE)

#define CATCH_CONFIG_MAIN
#include "private/ext/catch.hpp"

#include <measurement_kit/traceroute.hpp>

#include <iostream>

using namespace mk::traceroute;
using namespace mk;

TEST_CASE("Typical IPv4 traceroute usage") {

    std::string payload(256, '\0');
    Var<Reactor> reactor = Reactor::make();
    auto prober = Prober<AndroidProber>(true, 11829, reactor);
    auto ttl = 1;

    reactor->run_with_initial_event([&]() {
        prober.on_result([&](ProbeResult r) {
            std::cout << ttl << " " << r.interface_ip << " " << r.rtt
                      << " ms\n";
            if (r.get_meaning() != ProbeResultMeaning::TTL_EXCEEDED ||
                ttl >= 64) {
                reactor->stop();
                return;
            }
            prober.send_probe("8.8.8.8", 33434, ++ttl, payload, 1.0);
        });

        prober.on_timeout([&]() {
            std::cout << ttl << " *\n";
            if (ttl >= 64) {
                reactor->stop();
                return;
            }
            prober.send_probe("8.8.8.8", 33434, ++ttl, payload, 1.0);
        });

        prober.on_error([&](Error err) {
            std::cout << ttl << " error: " << err.what() << "\n";
            if (ttl >= 64) {
                reactor->stop();
                return;
            }
            prober.send_probe("8.8.8.8", 33434, ++ttl, payload, 1.0);
        });

        prober.send_probe("8.8.8.8", 33434, ttl, payload, 1.0);
    });
}

TEST_CASE("Check whether it works when destination sends reply") {

    std::string payload(256, '\0');
    Var<Reactor> reactor = Reactor::make();
    auto prober = Prober<AndroidProber>(true, 11829, reactor);
    auto ttl = 1;

    reactor->run_with_initial_event([&]() {
        prober.on_result([&](ProbeResult r) {
            std::cout << ttl << " " << r.interface_ip << " " << r.rtt
                      << " ms\n";
            if (r.get_meaning() != ProbeResultMeaning::TTL_EXCEEDED ||
                ttl >= 64) {
                reactor->stop();
                return;
            }
            prober.send_probe("208.67.222.222", 53, ++ttl, payload, 1.0);
        });

        prober.on_timeout([&]() {
            std::cout << ttl << " *\n";
            if (ttl >= 64) {
                reactor->stop();
                return;
            }
            prober.send_probe("208.67.222.222", 53, ++ttl, payload, 1.0);
        });

        prober.on_error([&](Error err) {
            std::cout << ttl << " error: " << err.what() << "\n";
            if (ttl >= 64) {
                reactor->stop();
                return;
            }
            prober.send_probe("208.67.222.222", 53, ++ttl, payload, 1.0);
        });

        prober.send_probe("208.67.222.222", 53, ttl, payload, 1.0);
    });
}

#else
int main(){}
#endif

#else
int main(){}
#endif
