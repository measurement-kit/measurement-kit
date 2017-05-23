// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_RETURN_TYPE_HPP
#define MEASUREMENT_KIT_COMMON_RETURN_TYPE_HPP

#include <type_traits>

namespace mk {

// unspecified template definition
template <typename T> class ReturnType;

// deduce return type of function
template <typename R, typename... A> class ReturnType<R(A...)> {
  public:
    using type = R;
};

// deduce return type of constant method
template <class C, typename R, typename... A>
class ReturnType<R (C::*)(A...) const>
    : public ReturnType<R(std::add_pointer<C>, A...)> {};

// deduce return type of lambda
template <typename L>
class ReturnType : public ReturnType<decltype(&L::operator())> {};

} // namespace
#endif
