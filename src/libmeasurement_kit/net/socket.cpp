// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "private/libevent/pollfd.hpp"         // for mk::libevent::pollfd
#include <cassert>                             // for ::assert
#include <event2/event.h>                      // for EV_READ, EV_WRITE
#include <measurement_kit/common/callback.hpp> // for mk::Callback
#include <measurement_kit/common/error.hpp>    // for mk::Error
#include <measurement_kit/common/logger.hpp>   // for mk::Logger
#include <measurement_kit/common/reactor.hpp>  // for mk::Reactor
#include <measurement_kit/common/var.hpp>      // for mk::Var
#include <measurement_kit/net/socket.hpp>      // for mk::pollin, mk::pollout

namespace mk {
namespace net {

void pollin(os_socket_t sockfd, double timeout, Reactor reactor,
            Var<Logger> logger, Callback<Error> &&callback) {
    libevent::pollfd(sockfd, EV_READ, timeout, reactor, [
        callback = std::move(callback), sockfd, logger
    ](Error err, short evflags) {
        assert((evflags & (EV_READ | EV_TIMEOUT)) != 0);
        logger->log(MK_LOG_DEBUG2, "pollfd: fd=%lld evflags=%d",
                    (long long)sockfd, evflags);
        callback(err);
    });
}

void pollout(os_socket_t sockfd, double timeout, Reactor reactor,
             Var<Logger> logger, Callback<Error> &&callback) {
    libevent::pollfd(sockfd, EV_WRITE, timeout, reactor, [
        callback = std::move(callback), sockfd, logger
    ](Error err, short evflags) {
        assert((evflags & (EV_WRITE | EV_TIMEOUT)) != 0);
        logger->log(MK_LOG_DEBUG2, "pollfd: fd=%lld evflags=%d",
                    (long long)sockfd, evflags);
        callback(err);
    });
}

} // namespace net
} // namespace mk
