// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../libevent/dns.hpp"

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
    libevent::query(dns_class, dns_type, name, cb, settings, reactor, logger);
}

} // namespace dns
} // namespace mk
