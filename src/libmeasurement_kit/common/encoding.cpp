// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/common/encoding.hpp"

#include "measurement_kit/vendor/mkdata.h"

namespace mk {

Error utf8_parse(const std::string &str) {
  mkdata_uptr d{mkdata_new_nonnull()};
  mkdata_set_v2(d.get(), (const uint8_t *)str.c_str(), str.size());
  if (!mkdata_contains_valid_utf8_v2(d.get())) {
    return IllegalSequenceError();
  }
  return NoError();
}

std::string base64_encode(std::string str) {
  mkdata_uptr d{mkdata_new_nonnull()};
  mkdata_movein_data(d, std::move(str));
  return mkdata_moveout_base64(d);
}

} // namespace mk
