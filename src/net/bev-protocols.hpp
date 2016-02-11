// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_NET_BEV_PROTOCOLS_HPP
#define SRC_NET_BEV_PROTOCOLS_HPP

#include <functional>
#include <string>

namespace mk {

class Error;
template <typename T> class Var;

namespace net {

class TransportBev;

void http_09_sendrecv(Var<TransportBev> tbev, std::string request,
                      std::function<void(Error)> callback,
                      std::string *response, double timeout);

} // namespace net
} // namespace mk
#endif
