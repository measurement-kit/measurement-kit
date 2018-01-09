// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_COMMON_ENCODING_HPP
#define SRC_LIBMEASUREMENT_KIT_COMMON_ENCODING_HPP

#include <measurement_kit/common/error.hpp>
#include <string>

namespace mk {

Error utf8_parse(const std::string &s);

std::string base64_encode(std::string s);

} // namespace mk
#endif
