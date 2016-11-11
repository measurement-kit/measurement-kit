// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_DNS_QUERY_HPP
#define SRC_LIBMEASUREMENT_KIT_DNS_QUERY_HPP

#include "../libevent/dns.hpp"

namespace mk {
namespace dns {
namespace cares {

void query(
        QueryClass dns_class,
        QueryType dns_type,
        std::string name,
        Callback<Error, Var<Message>> cb,
        Settings settings,
        Var<Reactor> reactor,
        Var<Logger> logger);

} // namespace cares
} // namespace dns
} // namespace mk
#endif
