// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <stdexcept>
#include <new>

#include <event2/event.h>
#include <event2/dns.h>

#include "src/common/check_connectivity.hpp"
#include <measurement_kit/common/logger.hpp>

namespace mk {

void CheckConnectivity::cleanup() // Idempotent cleanup function
{
    if (dnsbase != nullptr) {
        evdns_base_free(dnsbase, 0);
        dnsbase = nullptr;
    }
    if (evbase != nullptr) {
        event_base_free(evbase);
        evbase = nullptr;
    }
}

void CheckConnectivity::dns_callback(int result, char, int, int, void *,
                                     void *opaque) {
    auto that = static_cast<CheckConnectivity *>(opaque);

    switch (result) {
    case DNS_ERR_NONE:
    case DNS_ERR_FORMAT:
    case DNS_ERR_SERVERFAILED:
    case DNS_ERR_NOTEXIST:
    case DNS_ERR_NOTIMPL:
    case DNS_ERR_REFUSED:
    case DNS_ERR_TRUNCATED:
    case DNS_ERR_NODATA:
        that->is_up = true;
        break;
    default:
        that->is_up = false;
    }

    if (event_base_loopbreak(that->evbase) != 0) {
        throw std::runtime_error("Cannot exit from event loop");
    }
}

CheckConnectivity::CheckConnectivity() {
    if ((evbase = event_base_new()) == nullptr) {
        cleanup();
        throw std::bad_alloc();
    }

    if ((dnsbase = evdns_base_new(evbase, 1)) == nullptr) {
        cleanup();
        throw std::bad_alloc();
    }

    if (evdns_base_resolve_ipv4(dnsbase, "ebay.com", DNS_QUERY_NO_SEARCH,
                                dns_callback, this) == nullptr) {
        cleanup();
        throw std::runtime_error("cannot resolve 'ebay.com'");
    }

    if (event_base_dispatch(evbase) != 0) {
        cleanup();
        throw std::runtime_error("event_base_dispatch() failed");
    }

    cleanup();

    if (!is_up) {
        warn("network is down");
    }
}

} // namespace mk
