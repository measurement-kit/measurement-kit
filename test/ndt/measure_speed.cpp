// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "test/winsock.hpp"

#include "src/libmeasurement_kit/ext/catch.hpp"

#include <measurement_kit/common.hpp>
#include "src/libmeasurement_kit/ndt/measure_speed.hpp"

using namespace mk;
using namespace mk::ndt;

TEST_CASE("The constructor with snap-delay works as expected") {
    MeasureSpeed msp(42.0);
    REQUIRE(msp.previous == Approx(mk::time_now()));
    REQUIRE(msp.start_time == Approx(mk::time_now()));
    REQUIRE(msp.snap_delay == 42.0);
    REQUIRE(msp.total == 0);
}

TEST_CASE("The default constructor works as expected") {
    MeasureSpeed msp;
    REQUIRE(msp.previous == Approx(mk::time_now()));
    REQUIRE(msp.start_time == Approx(mk::time_now()));
    REQUIRE(msp.snap_delay == -1.0);
    REQUIRE(msp.total == 0);
}

TEST_CASE("MeasureSpeed::speed() computes correctly the speed") {
    MeasureSpeed msp;
    msp.total = 1000000;
    REQUIRE(msp.speed(msp.previous + 10.0) == Approx(800.0));
}

TEST_CASE("MeasureSpeed::speed() avoids division by 0 and negative results") {
    MeasureSpeed msp;
    msp.total = 1000000;
    REQUIRE(msp.speed(msp.previous) == Approx(0.0));
    REQUIRE(msp.speed(msp.previous - 1.0) == Approx(0.0));
}

TEST_CASE("MeasureSpeed::reset() correctly resets internals") {
    MeasureSpeed msp(42.0);
    double orig_start_time = msp.start_time;
    msp.total = 1000000;
    double ct = 7.0;
    msp.reset(ct);
    REQUIRE(msp.previous == ct);
    REQUIRE(msp.start_time == orig_start_time);
    REQUIRE(msp.snap_delay == 42.0);
    REQUIRE(msp.total == 0);
}

TEST_CASE("MeasureSpeed::maybe_speed() not called with negative snap-delay") {
    MeasureSpeed msp;
    msp.total = 1000000;
    double prev_t = msp.previous;
    msp.maybe_speed(msp.previous + 10.0, [](double, double) {
        REQUIRE(false); // Arriving here is an error
    });
    REQUIRE(msp.total == 1000000);
    REQUIRE(msp.previous == prev_t);
}

TEST_CASE("MeasureSpeed::maybe_speed() called only after delay is passed") {
    MeasureSpeed msp(7.0);
    double prev_t = msp.previous;
    msp.total = 1000000;
    msp.maybe_speed(msp.previous + 5.0, [](double, double) {
        REQUIRE(false); // Arriving here is an error
    });
    REQUIRE(msp.total == 1000000);
    REQUIRE(msp.previous == prev_t);
    msp.maybe_speed(msp.previous + 10.0, [](double elapsed, double speed) {
        REQUIRE(elapsed == Approx(10.0));
        REQUIRE(speed == Approx(800.0));
    });
    REQUIRE(msp.total == 0);
    REQUIRE(msp.previous == Approx(prev_t + 10.0));
}

TEST_CASE("The first argument passed to callback is since the beginning") {
    MeasureSpeed msp(7.0);
    msp.total = 1000000;
    msp.maybe_speed(msp.previous + 10.0, [](double elapsed, double speed) {
        REQUIRE(elapsed == Approx(10.0));
        REQUIRE(speed == Approx(800.0));
    });
    REQUIRE(msp.total == 0);
    msp.total = 1000000;
    msp.maybe_speed(msp.previous + 10.0, [](double elapsed, double speed) {
        REQUIRE(elapsed == Approx(20.0)); // Must grow
        REQUIRE(speed == Approx(800.0));
    });
}
