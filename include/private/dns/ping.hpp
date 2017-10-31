// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef PRIVATE_DNS_PING_HPP
#define PRIVATE_DNS_PING_HPP

#include "private/common/every.hpp"
#include "private/common/maybe.hpp"
#include "private/common/utils.hpp"
#include <measurement_kit/dns/query.hpp>

namespace mk {
namespace dns {

template <typename ResultsCollector, typename Callback>
void ping_nameserver(QueryClass dns_class, QueryType dns_type, std::string name,
                     double interval, Maybe<double> run_for, Settings settings,
                     SharedPtr<Reactor> reactor, SharedPtr<Logger> logger,
                     ResultsCollector collector, Callback callback) {
    if (run_for) {
        *run_for += time_now(); /* From relative to absolute timing */
    }
    mk::every(
          interval, reactor, callback,
          [=]() { return run_for && time_now() > *run_for; },
          [=]() {
              query(dns_class, dns_type, name,
                    [=](Error err, SharedPtr<Message> msg) { collector(err, msg); },
                    settings, reactor, logger);
          });
}

} // namespace dns
} // namespace mk
#endif
