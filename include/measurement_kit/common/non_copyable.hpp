// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_NON_COPYABLE_HPP
#define MEASUREMENT_KIT_COMMON_NON_COPYABLE_HPP

namespace mk {

class NonCopyable {
  public:
    NonCopyable(NonCopyable &) = delete;
    NonCopyable &operator=(NonCopyable &) = delete;
    NonCopyable() {}
};

} // namespace mk
#endif
