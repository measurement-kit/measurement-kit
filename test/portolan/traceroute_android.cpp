/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

//
// Tests for portolan/traceroute_android.{c,h}pp
//

// We use this on Android and compile this on all Linuxes
#ifdef __linux__

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <ight/portolan/traceroute_android.hpp>

#include <iostream>

using namespace ight::portolan::traceroute_android;

TEST_CASE("Typical IPv4 traceroute usage") {

    auto prober = Prober::open(true, 54321);
    auto ttl = 1;

    prober->on_result([prober, &ttl](ProbeResult r) {
        std::cout << ttl << " " << r.interface_ip << " " << r.rtt << " ms\n";
        if (r.get_meaning() != Meaning::TTL_EXCEEDED || ttl >= 64) {
            ight_break_loop();
            return;
        }
        prober->send_probe("130.192.16.172", 33434, ++ttl, "antani");
    });

    prober->on_timeout([prober, &ttl]() {
        std::cout << ttl << " *\n";
        if (ttl >= 64) {
            ight_break_loop();
            return;
        }
        prober->send_probe("130.192.16.172", 33434, ++ttl, "antani");
    });

    prober->on_error([prober, &ttl](std::runtime_error err) {
        std::cout << ttl << " error: " << err.what() << "\n";
        if (ttl >= 64) {
            ight_break_loop();
            return;
        }
        prober->send_probe("130.192.16.172", 33434, ++ttl, "antani");
    });

    prober->send_probe("130.192.16.172", 33434, ttl, "antani");
    ight_loop();

    // Clear self references caused by capture lists to avoid memleaks
    prober->close();
}
#endif
