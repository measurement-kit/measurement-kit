// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_NET_SOCKET_HPP
#define MEASUREMENT_KIT_NET_SOCKET_HPP

#include <cstdint>                             // for uintptr_t
#include <measurement_kit/common/callback.hpp> // for mk::Callback
#include <measurement_kit/common/error.hpp>    // for mk::Error
#include <measurement_kit/common/logger.hpp>   // for mk::Logger
#include <measurement_kit/common/reactor.hpp>  // for mk::Reactor
#include <measurement_kit/common/var.hpp>      // for mk::Var

namespace mk {
namespace net {

#ifdef _WIN32
using os_socket_t = uintptr_t;
#else
using os_socket_t = int;
#endif

void pollin(os_socket_t sockfd, double timeout, Reactor reactor,
            Var<Logger> logger, Callback<Error> &&callback);

void pollout(os_socket_t sockfd, double timeout, Reactor reactor,
             Var<Logger> logger, Callback<Error> &&callback);

} // namespace net
} // namespace mk
#endif
