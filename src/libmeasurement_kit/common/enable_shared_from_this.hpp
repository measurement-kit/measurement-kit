// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_COMMON_ENABLE_SHARED_FROM_THIS_HPP
#define SRC_LIBMEASUREMENT_KIT_COMMON_ENABLE_SHARED_FROM_THIS_HPP

#include <measurement_kit/common/shared_ptr.hpp>

namespace mk {

/// `EnableSharedFromThis` is a `std::enable_shared_from_this` wrapper that
/// wraps `std::shared_ptr` using `mk::SharedPtr`.
template <typename Type>
class EnableSharedFromThis : public std::enable_shared_from_this<Type> {
  public:
    SharedPtr<Type> shared_from_this() {
        return SharedPtr<Type>{
                std::enable_shared_from_this<Type>::shared_from_this()};
    }

  protected:
  private:
    // NO ATTRIBUTES HERE BY DESIGN. DO NOT ADD ATTRIBUTES HERE BECAUSE
    // DOING THAT CREATES THE RISK OF OBJECT SLICING.
};

} // namespace mk
#endif
