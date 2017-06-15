// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../libevent/dns_impl.hpp"

namespace mk {
namespace libevent {

extern "C" {
    void handle_resolve(int code, char type, int count, int ttl,
            void *addresses, void *opaque) {
        auto context = static_cast<QueryContext *>(opaque);
        dns_callback(code, type, count, ttl, addresses, context);
    }
}

void query(
        dns::NameServers ns,
        dns::QueryClass dns_class,
        dns::QueryType dns_type,
        std::string name,
        Settings settings,
        Var<Reactor> reactor,
        Var<Logger> logger,
        Callback<Error, Var<dns::Message>> cb) {
    query_impl(ns, dns_class, dns_type, name, settings, reactor, logger, cb);
}

} // namespace libevent
} // namespace mk
