// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_LEXICAL_CAST_HPP
#define MEASUREMENT_KIT_COMMON_LEXICAL_CAST_HPP

#include <measurement_kit/common/scalar.hpp>

namespace mk {

template <typename To, typename From> To lexical_cast(From f) {
    return Scalar{f}.as<To>();
}

template <typename To, typename From>
ErrorOr<To> lexical_cast_noexcept(From f) {
    return Scalar{f}.as_noexcept<To>();
}

} // namespace mk
#endif
