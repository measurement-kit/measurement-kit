// Public domain 2017, Simone Basso <bassosimone@gmail.com.

#include "test/winsock.hpp"

#include <measurement_kit/nettest.hpp>

#include <stdlib.h>

#include <iostream>

int main() {
    mk::nettest::routers::NoisyRouter router;
    mk::nettest::settings::WebConnectivitySettings settings;
    settings.inputs = {
        "https://slashdot.org/",
        "http://www.microsoft.com"
    };
    settings.log_level = settings.log_level_info;
    mk::nettest::Nettest nettest;
    if (!nettest.start_web_connectivity(settings, &router)) {
        std::clog << "ERROR: cannot start nettest" << std::endl;
        exit(1);
    }
    while (!nettest.is_done()) {
        nettest.route_next_event(&router);
    }
}
