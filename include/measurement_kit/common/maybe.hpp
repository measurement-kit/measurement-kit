// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_MAYBE_HPP
#define MEASUREMENT_KIT_COMMON_MAYBE_HPP

#include <stdexcept>
#include <type_traits>

namespace mk {

template <typename T> class Maybe {
  public:
    Maybe() {}
    Maybe(T &&t) : value_{std::move(t)}, valid_{true} {}

#define IMPL                                                                   \
    if (!valid_) {                                                             \
        throw std::runtime_error("Maybe is empty");                            \
    }                                                                          \
    return value_

    T &operator*() { IMPL; }

    const T &operator*() const { IMPL; }

#undef IMPL

    operator bool() const { return valid_; }

  private:
    T value_ = {};
    bool valid_ = false;
};

} // namespace mk
#endif
