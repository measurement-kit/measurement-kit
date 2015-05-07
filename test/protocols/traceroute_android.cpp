/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

/// Tests Android traceroute prober

// This is meant to run on Android but can run on all Linux systems
#ifdef __linux__

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <ight/protocols/traceroute_android.hpp>

#include <iostream>

using namespace ight::protocols::traceroute;

TEST_CASE("Typical IPv4 traceroute usage") {

    std::string payload(256, '\0');
    auto prober = Prober<AndroidProber>(true, 11829);
    auto ttl = 1;

    prober.on_result([&prober, &ttl, &payload](ProbeResult r) {
        std::cout << ttl << " " << r.interface_ip << " " << r.rtt << " ms\n";
        if (r.get_meaning() != ProbeResultMeaning::TTL_EXCEEDED || ttl >= 64) {
            ight_break_loop();
            return;
        }
        prober.send_probe("8.8.8.8", 33434, ++ttl, payload);
    });

    prober.on_timeout([&prober, &ttl, &payload]() {
        std::cout << ttl << " *\n";
        if (ttl >= 64) {
            ight_break_loop();
            return;
        }
        prober.send_probe("8.8.8.8", 33434, ++ttl, payload);
    });

    prober.on_error([&prober, &ttl, &payload](std::runtime_error err) {
        std::cout << ttl << " error: " << err.what() << "\n";
        if (ttl >= 64) {
            ight_break_loop();
            return;
        }
        prober.send_probe("8.8.8.8", 33434, ++ttl, payload);
    });

    prober.send_probe("8.8.8.8", 33434, ttl, payload);
    ight_loop();
}
#endif
