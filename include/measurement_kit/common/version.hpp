// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_VERSION_HPP
#define MEASUREMENT_KIT_COMMON_VERSION_HPP

#include <string>

// Note: we use semantic versioning (see: http://semver.org/)
#define MEASUREMENT_KIT_VERSION "0.3.8"

namespace mk {

std::string library_version();

} // namespace mk
#endif
