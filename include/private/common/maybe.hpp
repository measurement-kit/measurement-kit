// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef PRIVATE_COMMON_MAYBE_HPP
#define PRIVATE_COMMON_MAYBE_HPP

#include <stdexcept>
#include <type_traits>

namespace mk {

template <typename T> class Maybe {
  public:
    Maybe() {}
    Maybe(T &&t) : value_{std::move(t)}, valid_{true} {}

#define DETAIL                                                                   \
    if (!valid_) {                                                             \
        throw std::runtime_error("Maybe is empty");                            \
    }                                                                          \
    return value_

    T &operator*() { DETAIL; }

    const T &operator*() const { DETAIL; }

#undef DETAIL

    operator bool() const { return valid_; }

  private:
    T value_ = {};
    bool valid_ = false;
};

} // namespace mk
#endif
