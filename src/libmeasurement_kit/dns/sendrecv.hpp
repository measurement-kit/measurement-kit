// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_DNS_SENDRECV_HPP
#define SRC_LIBMEASUREMENT_KIT_DNS_SENDRECV_HPP

#include <measurement_kit/dns.hpp>

namespace mk {
namespace dns {

/*
 * Note: this API is generic enough that we may want to use part of it
 * to re-implement also parts of the Android traceroute code.
 *
 * In general, this API has very little to do with DNS and is more a
 * UDP thing that can be useful to any piece of code that sends a UDP
 * packet and waits for a response. So, perhaps, we may want to move
 * at least the engine of it in `measurement_kit/net`. I'd do that
 * when we need to share it, however, rather than doing it now.
 */

ErrorOr<Var<socket_t>> send(
        std::string nameserver,
        std::string port,
        std::string packet,
        Var<Logger> logger
);

void pollin(
        Var<socket_t> sock,
        Callback<Error> callback,
        Settings settings,
        Var<Reactor> reactor,
        Var<Logger> logger
);

ErrorOr<std::string> recv(
        Var<socket_t> sock,
        Var<Logger> logger
);

void sendrecv(
        std::string nameserver,
        std::string port,
        std::string packet,
        Callback<Error, std::string> callback,
        Settings settings,
        Var<Reactor> reactor,
        Var<Logger> logger
);

} // namespace dns
} // namespace mk
#endif
