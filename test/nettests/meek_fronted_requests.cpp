// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifdef ENABLE_INTEGRATION_TESTS
#define CATCH_CONFIG_MAIN
#include "../src/libmeasurement_kit/ext/catch.hpp"

#include "../nettests/utils.hpp"

using namespace mk::nettests;
using namespace mk;

TEST_CASE("Synchronous meek-fronted-requests test") {
    test::nettests::make_test<MeekFrontedRequestsTest>()
        .set_options("expected_body", "I\u2019m just a happy little web server.\n")
        .set_options("outer_host", "a0.awsstatic.com")
        .set_options("inner_host", "d2zfqthxsdq309.cloudfront.net")
        .run();
}

TEST_CASE("Asynchronous meek-fronted-requests test") {
    auto t = test::nettests::make_test<MeekFrontedRequestsTest>("meek_fronted_requests.txt");
    t.runnable->needs_input = true;
    test::nettests::run_async(t);
}

#else
int main() {}
#endif
