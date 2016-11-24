// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../dns/sendrecv_impl.hpp"

namespace mk {
namespace dns {

ErrorOr<Var<socket_t>> send(std::string nameserver, std::string port,
                            std::string packet, Var<Logger> logger) {
    return send_impl(nameserver, port, packet, logger);
}

void pollin(Var<socket_t> sock, Callback<Error> callback, Settings settings,
            Var<Reactor> reactor, Var<Logger> logger) {
    pollin_impl(sock, callback, settings, reactor, logger);
}

ErrorOr<std::string> recv(Var<socket_t> sock, Var<Logger> logger) {
    return recv_impl(sock, logger);
}

void sendrecv(std::string nameserver, std::string port, std::string packet,
              Callback<Error, std::string> callback, Settings settings,
              Var<Reactor> reactor, Var<Logger> logger) {
    sendrecv_impl(
        nameserver, port, packet, callback, settings, reactor, logger);
}

} // namespace dns
} // namespace mk
