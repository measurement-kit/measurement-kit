// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/common/encoding.hpp"

#include "measurement_kit/internal/vendor/mkdata.hpp"

namespace mk {

Error utf8_parse(const std::string &str) {
  if (!mk::data::contains_valid_utf8(str)) {
    return IllegalSequenceError();
  }
  return NoError();
}

std::string base64_encode(std::string str) {
  return mk::data::base64_encode(std::move(str));
}

std::string base64_encode_if_needed(std::string str) {
  if (utf8_parse(str) != NoError()) {
    return base64_encode(std::move(str));
  }
  return str;
}

} // namespace mk
