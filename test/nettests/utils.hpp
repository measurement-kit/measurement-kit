// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef TEST_NETTESTS_UTILS_HPP
#define TEST_NETTESTS_UTILS_HPP

#include "src/libmeasurement_kit/nettests/runnable.hpp"
#include "src/libmeasurement_kit/common/worker.hpp"
#include "src/libmeasurement_kit/ooni/collector_client.hpp"
#include "src/libmeasurement_kit/ooni/bouncer.hpp"

#include <measurement_kit/nettests.hpp>
#include "src/libmeasurement_kit/ooni/error.hpp"

#include <chrono>
#include <thread>

namespace test {
namespace nettests {

using with_test_cb = std::function<void(mk::nettests::BaseTest &)>;

static inline void run_test(mk::nettests::BaseTest &test) {
    test.run();
}

template <typename T> void with_test(with_test_cb &&lambda) {
    lambda(
          T{}.set_option("geoip_country_path", "country.mmdb")
                .add_annotation("continuous_integration", "true")
                .set_option("geoip_asn_path", "asn.mmdb")
                .set_option("net/ca_bundle_path", "cacert.pem")
                .set_verbosity(MK_LOG_INFO)
                .set_option("collector_base_url",
                            "https://ams-pg-test.ooni.org")
                .set_option("bouncer_base_url",
                             mk::ooni::bouncer::production_bouncer_url()));
    /*
     * Wait for the default tasks queue to empty, so we exit from the
     * process without still running detached threads and we don't leak
     * memory and, therefore, valgrind memcheck does not fail.
     *
     * See also `test/ooni/orchestrate.cpp`.
     */
    mk::Worker::default_tasks_queue()->wait_empty_();
}

template <typename T> void with_test(std::string s, with_test_cb &&lambda) {
    with_test<T>([ s = std::move(s),
                   lambda = std::move(lambda) ](mk::nettests::BaseTest & test) {
        lambda(test.add_input_filepath("./test/fixtures/" + s));
    });
}

static inline void
with_runnable(std::function<void(mk::nettests::Runnable &)> lambda) {
    mk::nettests::Runnable test;
    test.annotations["continuous_integration"] = "true";
    test.options["net/ca_bundle_path"] = "cacert.pem";
    test.options["collector_base_url"] = "https://ams-pg-test.ooni.org";
    test.options["bouncer_base_url"] =
          mk::ooni::bouncer::production_bouncer_url();
    /*
     * The `with_runnable` function is used for tests for which we do not
     * care to submit to a collector. So, disable the collector so that these
     * tests are going to be much faster than otherwise.
     */
    test.options["no_collector"] = 1;
    test.logger->set_verbosity(MK_LOG_INFO);
    lambda(test);
}

} // namespace nettests
} // namespace test
#endif
