// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifdef ENABLE_INTEGRATION_TESTS
#define CATCH_CONFIG_MAIN
#include "../src/libmeasurement_kit/ext/catch.hpp"
#include "../src/libmeasurement_kit/ooni/captive_portal.hpp"

#include "../nettests/utils.hpp"

using namespace mk::nettests;
using namespace mk;
using namespace mk::ooni;

//TEST_CASE("Synchronous captive portal test") {
//    test::nettests::make_test<CaptivePortalTest>()
//        .run();
//}
//
//TEST_CASE("Asynchronous captive portal test") {
//    test::nettests::run_async(
//        test::nettests::make_test<CaptivePortalTest>()
//    );
//}

TEST_CASE("gen_no_cp_f works") {
    input i;
    i["name"] = "Android Lollipop HTTP";
    i["url"] = "http://connectivitycheck.android.com/generate_204";
    i["status"] = "204";

    Var<http::Response> r{new http::Response};

    Var<no_cp_f_t> no_cp_f(new no_cp_f_t);
    gen_no_cp_f(i, no_cp_f);

    (*no_cp_f)(r);
}

#else
int main() {}
#endif
