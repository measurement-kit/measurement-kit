// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef PRIVATE_LIBEVENT_DNS_HPP
#define PRIVATE_LIBEVENT_DNS_HPP

#include <measurement_kit/dns.hpp>

namespace mk {
namespace libevent {

void query(
        dns::QueryClass dns_class,
        dns::QueryType dns_type,
        std::string name,
        Callback<Error, Var<dns::Message>> cb,
        Settings settings,
        Var<Reactor> reactor,
        Var<Logger> logger
);

Error dns_error(int code);

} // namespace libevent
} // namespace mk
#endif
