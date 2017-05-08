// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_LIBEVENT_DNS_HPP
#define SRC_LIBMEASUREMENT_KIT_LIBEVENT_DNS_HPP

#include <measurement_kit/dns.hpp>

namespace mk {
namespace libevent {

void query(
        dns::NameServers ns,
        dns::QueryClass dns_class,
        dns::QueryType dns_type,
        std::string name,
        Settings settings,
        Var<Reactor> reactor,
        Var<Logger> logger,
        Callback<Error, Var<dns::Message>> cb
);

Error dns_error(int code);

} // namespace libevent
} // namespace mk
#endif
