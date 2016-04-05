// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_NET_EVBUFFER_HPP
#define SRC_NET_EVBUFFER_HPP

#include <event2/buffer.h>
#include <measurement_kit/common.hpp>
#include <memory>

namespace mk {
namespace net {

template<MK_MOCK(evbuffer_new), MK_MOCK(evbuffer_free)>
Var<evbuffer> make_shared_evbuffer() {
    evbuffer *p = evbuffer_new();
    if (p == nullptr) {
        throw std::bad_alloc();
    }
    return Var<evbuffer>(p, [](evbuffer *x) { evbuffer_free(x); });
}

} // namespace net
} // namespace mk
#endif
