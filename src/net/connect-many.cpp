// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/net/connect-many.hpp"

namespace mk {
namespace net {

void connect_many(std::string address, int port, int num,
        ConnectManyCb callback, Settings settings, Var<Logger> logger,
        Poller *poller) {
    connect_many_<net::connect>(connect_many_make(address, port, num,
            callback, settings, logger, poller));
}

} // namespace net
} // namespace mk
