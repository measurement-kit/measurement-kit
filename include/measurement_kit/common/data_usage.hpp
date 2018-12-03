// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_DATA_USAGE_HPP
#define MEASUREMENT_KIT_COMMON_DATA_USAGE_HPP

#include <stdint.h>

namespace mk {

// `DataUsage` provides information about the data sent and received
// by a specific reactor. Data usage information must be updated by code that
// deals with networking at low level and its accuracy is not 100%,
// e.g. we don't know exactly how many bytes were sent or received by
// an SSL connection, and we don't see retransmissions as well.
class DataUsage {
  public:
    // `down` is the amount of bytes downloaded so far.
    uint64_t down = 0;

    // `up` is the amount of bytes uploaded so far.
    uint64_t up = 0;
};

} // namespace mk
#endif
