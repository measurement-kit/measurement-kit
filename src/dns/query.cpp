// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/dns/query_impl.hpp"
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
        std::string name, Callback<Error, Message> cb,
        Settings settings, Var<Reactor> reactor) {
    query_impl (dns_class, dns_type, name, cb, settings, reactor);
}

} // namespace dns
} // namespace mk
