// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_COMMON_CONSTRAINTS_HPP
#define MEASUREMENT_KIT_COMMON_CONSTRAINTS_HPP

namespace mk {

struct NonMovable {
    NonMovable(NonMovable &&) = delete;
    NonMovable &operator=(NonMovable &&) = delete;
    NonMovable() {}
};

struct NonCopyable {
    NonCopyable(NonCopyable &) = delete;
    NonCopyable &operator=(NonCopyable &) = delete;
    NonCopyable() {}
};

} // namespace mk
#endif
