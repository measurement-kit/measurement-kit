// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_COMMON_CALLBACK_HPP
#define SRC_LIBMEASUREMENT_KIT_COMMON_CALLBACK_HPP

#include <functional>

namespace mk {

/// \brief `Callback` is syntactic sugar for writing callback functions. In
/// general, we try to use `Callback` in the code to name functions that will
/// be called _after_ the function to which they have been passed has
/// returned. That is, the function is supposed to schedule their deferred
/// execution but should not execute it directly.
///
/// \since v0.2.0.
template <typename... T> using Callback = std::function<void(T...)>;

} // namespace
#endif
