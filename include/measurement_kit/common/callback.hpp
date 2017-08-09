// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_CALLBACK_HPP
#define MEASUREMENT_KIT_COMMON_CALLBACK_HPP

#include <functional>

namespace mk {

template <typename... T> using Callback = std::function<void(T...)>;

} // namespace
#endif
