// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_SAFE_HPP
#define MEASUREMENT_KIT_COMMON_SAFE_HPP

#include <algorithm>   // for std::move
#include <stdexcept>   // for std::runtime_error

namespace mk {

template <typename Pointer> class Safe {
  public:
    Safe(Pointer &&pointer) : ptr_{std::move(pointer)} {}

    auto operator->() const {
        if (!ptr_) {
            throw std::runtime_error("null pointer");
        }
        return ptr_.get();
    }

  private:
    Pointer ptr_;
};

} // namespace mk
#endif
