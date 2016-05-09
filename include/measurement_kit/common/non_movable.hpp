// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_NON_MOVABLE_HPP
#define MEASUREMENT_KIT_COMMON_NON_MOVABLE_HPP

namespace mk {

class NonMovable {
  public:
    NonMovable(NonMovable &&) = delete;
    NonMovable &operator=(NonMovable &&) = delete;
    NonMovable() {}
};

} // namespace mk
#endif
