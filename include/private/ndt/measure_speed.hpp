// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef PRIVATE_NDT_MEASURE_SPEED_HPP
#define PRIVATE_NDT_MEASURE_SPEED_HPP

#include "private/common/utils.hpp"
#include <measurement_kit/common/callback.hpp>

namespace mk {
namespace ndt {

class MeasureSpeed {
  public:
    const double start_time = mk::time_now();
    double previous = start_time;
    double snap_delay = -1.0;
    size_t total = 0;

    MeasureSpeed(double delay) {
        snap_delay = delay;
    }
    MeasureSpeed() {}

    double speed(double ct) const {
        double elapsed = ct - previous;
        if (elapsed <= 0.0) {
            return 0.0;
        }
        return (total * 8.0) / 1000.0 / elapsed;
    }

    double speed() const {
        return speed(mk::time_now());
    }

    void reset(double ct) {
        previous = ct;
        total = 0;
    }

    void maybe_speed(double ct, Callback<double, double> cb) {
        if (snap_delay > 0.0 and ct - previous >= snap_delay) {
            cb(ct - start_time, speed(ct));
            reset(ct);
        }
    }

    void maybe_speed(Callback<double, double> cb) {
        maybe_speed(mk::time_now(), cb);
    }
};

} // namespace ndt
} // namespace mk
#endif
