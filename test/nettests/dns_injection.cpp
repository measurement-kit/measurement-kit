// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "test/winsock.hpp"

#include "include/private/catch.hpp"

#include "utils.hpp"

using namespace mk::nettests;

TEST_CASE("Synchronous dns-injection test") {
    test::nettests::with_test<DnsInjectionTest>(
          "hosts.txt",
          [](BaseTest &test) { test.set_option("dns/timeout", "0.1").run(); });
}
