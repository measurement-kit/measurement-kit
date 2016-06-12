// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_OONI_TEMPLATES_HPP
#define MEASUREMENT_KIT_OONI_TEMPLATES_HPP

#include <measurement_kit/common.hpp>
#include <measurement_kit/dns.hpp>
#include <measurement_kit/report.hpp>

namespace mk {
namespace ooni {
namespace templates {

void dns_query(Var<Entry> entry, dns::QueryType, dns::QueryClass,
               std::string query_name, std::string nameserver,
               Callback<dns::Message>, Settings, Var<Reactor>, Var<Logger>);

} // namespace templates
} // namespace mk
} // namespace ooni
#endif
