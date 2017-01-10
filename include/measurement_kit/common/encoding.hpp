// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_ENCODING_HPP
#define MEASUREMENT_KIT_COMMON_ENCODING_HPP

#include <measurement_kit/common/error.hpp>

namespace mk {

Error is_valid_utf8_string(const std::string &s);

std::string base64_encode(std::string s);

} // namespace mk
#endif
