// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_SAFE_HPP
#define MEASUREMENT_KIT_COMMON_SAFE_HPP

#include <algorithm> // for std::move, std::forward
#include <memory>    // for std::make_shared, std::shared_ptr
#include <stdexcept> // for std::runtime_error

namespace mk {

template <typename Pointer> class Safe {
  public:
    Safe(Pointer &&pointer) : pointer{std::move(pointer)} {}

    auto operator-> () const {
        if (!pointer) {
            throw std::runtime_error("null pointer");
        }
        return pointer.get();
    }

    // Design note: this is public so to allow the programmer to perform
    // possibly nasty operations on the underlying pointer without costing
    // us too much API wrapping effort. A common use case is to reset the
    // pointer once an object has become invalid. This, coupled with the
    // presence of `null pointer` exceptions allows to program better.
    Pointer pointer;
};

template <typename T, typename... A>
Safe<std::shared_ptr<T>> make_shared_safe(A &&... a) {
    return Safe<std::shared_ptr<T>>{std::make_shared<T>(std::forward<A>(a)...)};
}

} // namespace mk
#endif
