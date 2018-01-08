// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_NDT_UTILS_HPP
#define SRC_LIBMEASUREMENT_KIT_NDT_UTILS_HPP

#include "../ndt/internal.hpp"

namespace mk {
namespace ndt {
namespace utils {

report::Entry compute_ping(report::Entry &test_s2c, SharedPtr<Logger> logger);

report::Entry compute_speed(report::Entry &sender_or_receiver_data,
                            const char *speed_type, SharedPtr<Logger> logger);

report::Entry compute_simple_stats(report::Entry &entry, SharedPtr<Logger> logger);

report::Entry compute_advanced_stats(report::Entry &entry, SharedPtr<Logger> logger);

} // namespace utils
} // namespace ndt
} // namespace mk

#endif
