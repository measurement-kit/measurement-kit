// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef TEST_NETTESTS_UTILS_HPP
#define TEST_NETTESTS_UTILS_HPP

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
        .set_options("bouncer_base_url",
                mk::ooni::bouncer::testing_bouncer_url());
}

template <typename T> mk::nettests::BaseTest make_test(std::string s) {
    return make_test<T>().set_input_filepath("./test/fixtures/" + s);
}

static inline void run_async(mk::nettests::BaseTest test) {
    volatile bool done = false;
    test.start([&]() { done = true; });
    do {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    } while (!done);
}

static inline void
with_runnable(std::function<void(mk::nettests::Runnable &)> lambda) {
    mk::nettests::Runnable test;
    test.options["bouncer_base_url"] =
        mk::ooni::bouncer::testing_bouncer_url();
    test.use_bouncer = false;
    test.logger->set_verbosity(MK_LOG_INFO);
    lambda(test);
}

} // namespace nettests
} // namespace test
#endif
