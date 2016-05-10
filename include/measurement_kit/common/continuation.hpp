// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_CONTINUATION_HPP
#define MEASUREMENT_KIT_COMMON_CONTINUATION_HPP

#include <measurement_kit/common/callback.hpp>

namespace mk {

template <typename... T>
using Continuation = std::function<void(Callback<T...>)>;

} // namespace mk
#endif
