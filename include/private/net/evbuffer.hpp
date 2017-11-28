// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef PRIVATE_NET_EVBUFFER_HPP
#define PRIVATE_NET_EVBUFFER_HPP

#include "private/common/mock.hpp"

#include <measurement_kit/common.hpp>

#include <event2/buffer.h>

namespace mk {
namespace net {

template<MK_MOCK(evbuffer_new), MK_MOCK(evbuffer_free)>
SharedPtr<evbuffer> make_shared_evbuffer() {
    evbuffer *p = evbuffer_new();
    if (p == nullptr) {
        throw std::bad_alloc();
    }
    return SharedPtr<evbuffer>{std::shared_ptr<evbuffer>{
        p, [](evbuffer *x) { evbuffer_free(x); }}};
}

} // namespace net
} // namespace mk
#endif
