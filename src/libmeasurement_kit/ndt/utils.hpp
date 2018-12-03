// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_NDT_UTILS_HPP
#define SRC_LIBMEASUREMENT_KIT_NDT_UTILS_HPP

#include "src/libmeasurement_kit/ndt/internal.hpp"

namespace mk {
namespace ndt {
namespace utils {

nlohmann::json
compute_simple_stats_throws(nlohmann::json &entry, SharedPtr<Logger> logger);

nlohmann::json
compute_advanced_stats_throws(nlohmann::json &entry, SharedPtr<Logger> logger);

} // namespace utils
} // namespace ndt
} // namespace mk

#endif
