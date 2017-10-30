// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifdef ENABLE_INTEGRATION_TESTS

#define CATCH_CONFIG_MAIN
#include "private/ext/catch.hpp"

#include "utils.hpp"

using namespace mk::nettests;

TEST_CASE("Synchronous tcp-connect test") {
    test::nettests::with_test<TcpConnectTest>("hosts.txt", [](BaseTest &test) {
        test.set_option("port", "80").run();
    });
}

#else
int main() {}
#endif
