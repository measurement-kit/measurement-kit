// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../libevent/dns.hpp"
#include "resolver.hpp"

namespace mk {
namespace dns {

void query(
        QueryClass dns_class,
        QueryType dns_type,
        std::string name,
        Callback<Error, Var<Message>> cb,
        Settings settings,
        Var<Reactor> reactor,
        Var<Logger> logger) {
    std::string engine = settings.get("dns/engine", std::string("libevent"));
    if (engine == "libevent") {
        libevent::query(dns_class, dns_type, name, cb, settings, reactor, logger);
    } else if (engine == "system") {
        system_resolver(dns_class, dns_type, name, cb, settings, reactor, logger);
    } else {
        reactor->call_soon([cb]() {
                cb(InvalidDnsEngine(), nullptr);
        });
    }
}

} // namespace dns
} // namespace mk
