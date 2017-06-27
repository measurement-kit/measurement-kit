// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "../src/libmeasurement_kit/ext/catch.hpp"

#include "../nettests/utils.hpp"

using namespace mk::nettests;

#ifdef ENABLE_INTEGRATION_TESTS

TEST_CASE("Synchronous dns-injection test") {
    test::nettests::make_test<DnsInjectionTest>("hosts.txt")
        .set_options("dns/timeout", "0.1")
        .run();
}

TEST_CASE("Asynchronous dns-injection test") {
    test::nettests::run_async(
        test::nettests::make_test<DnsInjectionTest>("hosts.txt")
            .set_options("dns/timeout", "0.1")
    );
}

#endif

// Note: we don't repeat this test for every type of nettest
TEST_CASE("Make sure that set_output_filepath() works") {
    /*
       Note: must also set valid input file path otherwise the constructor
       called inside create_test_() throws an exception
    */
    auto runnable = DnsInjectionTest{}
                        .set_input_filepath("test/fixtures/hosts.txt")
                        .set_output_filepath("foo.txt")
                        .runnable;
    REQUIRE(runnable->output_filepath == "foo.txt");
}
