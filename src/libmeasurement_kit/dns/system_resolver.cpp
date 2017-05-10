// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../dns/system_resolver_impl.hpp"

namespace mk {
namespace dns {

void system_resolver(QueryClass dns_class, QueryType dns_type, std::string name,
                     Callback<Error, Var<Message>> cb, Settings settings,
                     Var<Reactor> reactor, Var<Logger> logger) {
    system_resolver_impl(dns_class, dns_type, name, cb, settings, reactor,
            logger);
}

} // namespace dns
} // namespace mk
