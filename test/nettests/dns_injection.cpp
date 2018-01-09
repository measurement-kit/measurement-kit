// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/libmeasurement_kit/ext/catch.hpp"

#include "utils.hpp"

using namespace mk::nettests;

#ifdef ENABLE_INTEGRATION_TESTS

TEST_CASE("Synchronous dns-injection test") {
    test::nettests::with_test<DnsInjectionTest>(
          "hosts.txt",
          [](BaseTest &test) { test.set_option("dns/timeout", "0.1").run(); });
}

#endif

// Note: we don't repeat this test for every type of nettest
TEST_CASE("Make sure that set_output_filepath() works") {
    /*
       Note: must also set valid input file path otherwise the constructor
       called inside create_test_() throws an exception
    */
    auto runnable = DnsInjectionTest{}
                        .add_input_filepath("test/fixtures/hosts.txt")
                        .set_output_filepath("foo.txt")
                        .runnable;
    REQUIRE(runnable->output_filepath == "foo.txt");
}
