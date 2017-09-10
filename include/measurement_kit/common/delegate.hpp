// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_DELEGATE_HPP
#define MEASUREMENT_KIT_COMMON_DELEGATE_HPP

#include <cstddef>
#include <functional>

namespace mk {

template <typename... T> using Delegate = std::function<void(T...)>;

} // namespace
#endif
