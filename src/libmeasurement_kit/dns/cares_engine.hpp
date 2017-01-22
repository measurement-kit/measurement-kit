// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_CARES_ENGINE_HPP
#define SRC_LIBMEASUREMENT_KIT_CARES_ENGINE_HPP

#include <measurement_kit/dns.hpp>

namespace mk {
namespace dns {

void cares_engine_query(
        QueryClass dns_class,
        QueryType dns_type,
        std::string name,
        Callback<Error, Var<Message>> cb,
        Settings settings,
        Var<Reactor> reactor,
        Var<Logger> logger
);

} // namespace dns
} // namespace mk
#endif
