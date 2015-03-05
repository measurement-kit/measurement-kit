/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

//
// Tests for portolan/traceroute_android.hpp
//

// We use this on Android and compile this on all Linuxes
#ifdef __linux__

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <ight/portolan/traceroute_android.hpp>

#include <iostream>

#include <poll.h>

using namespace ight::portolan::traceroute_android;

TEST_CASE("Basic IPv4 traceroute functionality") {
    event_base *evbase = event_base_new();
    if (evbase == NULL)
        throw std::bad_alloc();

    SharedPointer<Prober> prober = Prober::open(true, 54321, evbase);
    int ttl = 1;

    prober->on_result([prober, &evbase, &ttl](ProbeResult r) {
        std::cout << ttl << " " << r.interface_ip <<  " (" << r.interface_ip
                  << ") " << r.rtt << " ms\n";
        if (r.get_meaning() != Meaning::TTL_EXCEEDED || ttl >= 64) {
            event_base_loopbreak(evbase);
            return;
        }
        prober->send_probe("130.192.16.172", 33434, ++ttl, "antani");
    });

    prober->on_timeout([prober, &ttl]() {
        std::cout << ttl << " *\n";
        prober->send_probe("130.192.16.172", 33434, ++ttl, "antani");
    });

    prober->on_error([prober, &evbase, &ttl](std::runtime_error err) {
        std::cout << "Error occurred: " << err.what() << "\n";
        if (ttl >= 64) {
            event_base_loopbreak(evbase);
        }
        prober->send_probe("130.192.16.172", 33434, ++ttl, "antani");
    });

    prober->send_probe("130.192.16.172", 33434, ttl, "antani");
    event_base_dispatch(evbase);

    prober->close();
    event_base_free(evbase);
}

#endif  // __linux__
