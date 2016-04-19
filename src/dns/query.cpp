// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/dns/query.hpp"
struct evdns_base;

namespace mk {
namespace dns {

extern "C" {
    void handle_resolve(int code, char type, int count, int ttl,
            void *addresses, void *opaque) {
        auto context = static_cast<QueryContext *>(opaque);
        dns_callback(code, type, count, ttl, addresses, context);
    }
}

void query (QueryClass dns_class, QueryType dns_type,
        std::string name, Callback<Message> cb,
        Settings settings, Poller *poller) {
    query_debug (dns_class, dns_type, name, cb, settings, poller);
}

} // namespace dns
} // namespace mk
