// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_VENDOR_SOLE_HPP
#define SRC_LIBMEASUREMENT_KIT_VENDOR_SOLE_HPP

#include <string>

namespace mk {
namespace sole {

class uuid {
  public:
    std::string str();
    uint64_t ab;
    uint64_t cd;
};

uuid uuid4();

} // namespace sole
} // namespace mk
#endif
