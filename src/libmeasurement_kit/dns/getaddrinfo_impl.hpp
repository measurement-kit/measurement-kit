// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_DNS_GETADDRINFO_IMPL_HPP
#define SRC_LIBMEASUREMENT_KIT_DNS_GETADDRINFO_IMPL_HPP

#include <cassert>

#include "../dns/getaddrinfo.hpp"

namespace mk {
namespace dns {

template <MK_MOCK_ANONYMOUS_NAMESPACE(getaddrinfo),
          MK_MOCK_ANONYMOUS_NAMESPACE(freeaddrinfo)>
ErrorOr<Var<addrinfo>> getaddrinfo_impl(const char *hostname, const char *port,
                                        addrinfo *hints, Var<Logger> logger) {
    // Adapted from src/libmeasurement_kit/libevent/dns_impl.hpp
    addrinfo *ai = nullptr;
    int err = getaddrinfo(hostname, port, hints, &ai);
    if (err) {
        logger->warn("getaddrinfo() failed: %s", gai_strerror(err));
        return eai_to_error(err);
    }
    assert(ai != nullptr && ai->ai_next == nullptr);
    return Var<addrinfo>(ai, [](addrinfo *p) {
        if (p) {
            freeaddrinfo(p);
        }
    });
}

} // namespace dns
} // namespace mk
#endif
