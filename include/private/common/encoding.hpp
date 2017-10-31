// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef PRIVATE_COMMON_ENCODING_HPP
#define PRIVATE_COMMON_ENCODING_HPP

#include <measurement_kit/common/error.hpp>
#include <string>

namespace mk {

Error utf8_parse(const std::string &s);

std::string base64_encode(std::string s);

} // namespace mk
#endif
