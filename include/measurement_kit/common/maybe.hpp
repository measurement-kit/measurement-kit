// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_MAYBE_HPP
#define MEASUREMENT_KIT_COMMON_MAYBE_HPP

#include <type_traits>
#include <stdexcept>

namespace mk {

template <typename T> class Maybe {
  public:
    Maybe() {}
    Maybe(T &&t) : value_{std::move(t)}, valid_{true} {}

    typename std::add_lvalue_reference<T>::type operator*() {
        if (!valid_) {
            throw std::runtime_error("Maybe is empty");
        }
        return value_;
    }

    operator bool() const {
        return valid_;
    }

  private:
    T value_ = {};
    bool valid_ = false;
};

} // namespace mk
#endif
