// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../dns/system_resolver_impl.hpp"

namespace mk {
namespace dns {

void system_resolver(NameServers ns, QueryClass dns_class, QueryType dns_type,
                     std::string name, Settings settings, Var<Reactor> reactor,
                     Var<Logger> logger, Callback<Error, Var<Message>> cb) {
    system_resolver_impl(ns, dns_class, dns_type, name, settings, reactor,
                         logger, cb);
}

} // namespace dns
} // namespace mk
