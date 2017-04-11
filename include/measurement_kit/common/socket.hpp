// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_COMMON_SOCKET_HPP
#define MEASUREMENT_KIT_COMMON_SOCKET_HPP

#include <measurement_kit/common/aaa_base.hpp>

namespace mk {

#ifdef _WIN32
using socket_t = uintptr_t;
#else
using socket_t = int;
#endif

} // namespace mk
#endif
