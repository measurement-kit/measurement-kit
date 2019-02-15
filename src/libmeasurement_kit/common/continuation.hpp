// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_COMMON_CONTINUATION_HPP
#define SRC_LIBMEASUREMENT_KIT_COMMON_CONTINUATION_HPP

#include "src/libmeasurement_kit/common/callback.hpp"

namespace mk {

/// \brief `Continuation` is a function that will call another function
/// when complete. In several places we need to return code to be executed
/// later, perhaps in parallel with other code. In such cases, we usually
/// return a function that captures relevant parameters and receives in
/// input a callback to be called later when done. The `Continuation` alias
/// allows to express the returned function more compactly.
///
/// \since v0.2.0.
template <typename... T>
using Continuation = std::function<void(Callback<T...>)>;

} // namespace mk
#endif
