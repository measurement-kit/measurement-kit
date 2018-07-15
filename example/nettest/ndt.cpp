// Public domain 2017, Simone Basso <bassosimone@gmail.com.

#include "test/winsock.hpp"

#include <measurement_kit/nettest.hpp>

#include <stdlib.h>

#include <iostream>

int main() {
    mk::nettest::routers::NoisyRouter router;
    mk::nettest::settings::NdtSettings settings;
    settings.log_level = mk::nettest::log_levels::info;
    mk::nettest::Nettest nettest;
    if (!nettest.start_ndt(settings, &router)) {
        std::clog << "ERROR: cannot start nettest" << std::endl;
        exit(1);
    }
    while (!nettest.is_done()) {
        nettest.route_next_event(&router);
    }
}
