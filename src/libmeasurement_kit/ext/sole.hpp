// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
// =============================================================
// Derivative work of r-lyeh/sole@c61c49f10d.
// See NOTICE for original license.
#ifndef SRC_LIBMEASUREMENT_KIT_EXT_SOLE_HPP
#define SRC_LIBMEASUREMENT_KIT_EXT_SOLE_HPP

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
