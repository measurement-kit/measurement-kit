// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

/// Tests Android traceroute prober

// This is meant to run on Android but can run on all Linux systems
#ifdef __linux__

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <iostream>
#include <measurement_kit/traceroute.hpp>

using namespace mk::traceroute;
using namespace mk;

TEST_CASE("Typical IPv4 traceroute usage") {

    std::string payload(256, '\0');
    auto prober = Prober<AndroidProber>(true, 11829);
    auto ttl = 1;

    loop_with_initial_event_and_connectivity([&]() {
        prober.on_result([&prober, &ttl, &payload](ProbeResult r) {
            std::cout << ttl << " " << r.interface_ip << " " << r.rtt
                      << " ms\n";
            if (r.get_meaning() != ProbeResultMeaning::TTL_EXCEEDED ||
                ttl >= 64) {
                break_loop();
                return;
            }
            prober.send_probe("8.8.8.8", 33434, ++ttl, payload, 1.0);
        });

        prober.on_timeout([&prober, &ttl, &payload]() {
            std::cout << ttl << " *\n";
            if (ttl >= 64) {
                break_loop();
                return;
            }
            prober.send_probe("8.8.8.8", 33434, ++ttl, payload, 1.0);
        });

        prober.on_error([&prober, &ttl, &payload](Error err) {
            std::cout << ttl << " error: " << err.what() << "\n";
            if (ttl >= 64) {
                break_loop();
                return;
            }
            prober.send_probe("8.8.8.8", 33434, ++ttl, payload, 1.0);
        });

        prober.send_probe("8.8.8.8", 33434, ttl, payload, 1.0);
    });
}

TEST_CASE("Check whether it works when destination sends reply") {

    std::string payload(256, '\0');
    auto prober = Prober<AndroidProber>(true, 11829);
    auto ttl = 1;

    loop_with_initial_event_and_connectivity([&]() {
        prober.on_result([&prober, &ttl, &payload](ProbeResult r) {
            std::cout << ttl << " " << r.interface_ip << " " << r.rtt
                      << " ms\n";
            if (r.get_meaning() != ProbeResultMeaning::TTL_EXCEEDED ||
                ttl >= 64) {
                break_loop();
                return;
            }
            prober.send_probe("208.67.222.222", 53, ++ttl, payload, 1.0);
        });

        prober.on_timeout([&prober, &ttl, &payload]() {
            std::cout << ttl << " *\n";
            if (ttl >= 64) {
                break_loop();
                return;
            }
            prober.send_probe("208.67.222.222", 53, ++ttl, payload, 1.0);
        });

        prober.on_error([&prober, &ttl, &payload](Error err) {
            std::cout << ttl << " error: " << err.what() << "\n";
            if (ttl >= 64) {
                break_loop();
                return;
            }
            prober.send_probe("208.67.222.222", 53, ++ttl, payload, 1.0);
        });

        prober.send_probe("208.67.222.222", 53, ttl, payload, 1.0);
    });
}

#else
int main() { return 0; }
#endif
