// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef TEST_NETTESTS_UTILS_HPP
#define TEST_NETTESTS_UTILS_HPP

#include "private/nettests/runnable.hpp"

#include <measurement_kit/nettests.hpp>
#include <measurement_kit/ooni.hpp>

#include <chrono>
#include <thread>

namespace test {
namespace nettests {

template <typename T> mk::nettests::BaseTest make_test() {
    return T{}
        .set_options("geoip_country_path", "GeoIP.dat")
        .set_options("geoip_asn_path", "GeoIPASNum.dat")
        .set_verbosity(MK_LOG_INFO)
        /*
         * FIXME: the testing bouncer is not working. So use the testing
         * collector with the production bouncer.
         */
        .set_options("collector_base_url",
                mk::ooni::collector::testing_collector_url())
        .set_options("bouncer_base_url",
                mk::ooni::bouncer::production_bouncer_url());
}

template <typename T> mk::nettests::BaseTest make_test(std::string s) {
    return make_test<T>().set_input_filepath("./test/fixtures/" + s);
}

static inline void
with_runnable(std::function<void(mk::nettests::Runnable &)> lambda) {
    mk::nettests::Runnable test;
    // FIXME: see above comment regarding collector and bouncer
    test.options["collector_base_url"] =
        mk::ooni::collector::testing_collector_url();
    test.options["bouncer_base_url"] =
        mk::ooni::bouncer::production_bouncer_url();
    test.options["no_collector"] = 1; // Speed up these tests
    test.logger->set_verbosity(MK_LOG_INFO);
    lambda(test);
}

} // namespace nettests
} // namespace test
#endif
