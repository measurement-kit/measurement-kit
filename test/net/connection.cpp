// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include "src/net/connection.hpp"
#include <measurement_kit/net.hpp>

using namespace mk;
using namespace mk::net;

TEST_CASE("connect() iterates over all the available addresses") {
    // TODO: remove
    // Test cancelled because not applicable anymore
}

TEST_CASE("It is possible to use Connection with a custom poller") {
    // TODO: remove
    // Test superseded by similar test in test/net/transport.cpp
}
