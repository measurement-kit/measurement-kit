// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../libevent/listen.hpp"

using namespace mk;
using namespace mk::libevent;

extern "C" {

void mk_listener_cb(
        evconnlistener *, evutil_socket_t s, sockaddr *, int, void *o) {
    (*static_cast<ListenInternalCb *>(o))(s);
    // Note: persistent callback, would be stupid to `delete` it!
}

} // extern "C"
