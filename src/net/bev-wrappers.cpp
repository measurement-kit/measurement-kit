// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <event2/bufferevent.h>
#include <openssl/err.h>
#include <string>
#include <string.h>
#include "src/net/bev-wrappers.hpp"

namespace mk {
namespace net {

std::string event_string(short what) {
    std::string descr = "";
    if ((what & BEV_EVENT_READING) != 0) descr += "reading ";
    if ((what & BEV_EVENT_WRITING) != 0) descr += "writing ";
    if ((what & BEV_EVENT_CONNECTED) != 0) descr += "connected ";
    if ((what & BEV_EVENT_EOF) != 0) descr += "eof ";
    if ((what & BEV_EVENT_TIMEOUT) != 0) descr += "timeout ";
    if ((what & BEV_EVENT_ERROR) != 0) descr += "error ";
    return descr;
}

std::string strerror(Var<bufferevent> bev, short what) {
    std::string descr = event_string(what);
    if ((what & BEV_EVENT_ERROR) != 0 && errno != 0) {
        descr += "(errno: ";
        descr += ::strerror(errno);
        descr += ") ";
    }
    unsigned long error;
    char buffer[1024];
    for (;;) {
        if ((error = mk::net::bufferevent_get_openssl_error(bev)) == 0) break;
        ERR_error_string_n(error, buffer, sizeof(buffer));
        descr += "(openssl: ";
        descr += buffer;
        descr += ")";
    }
    return descr;
}

} // namespace net
} // namespace mk
