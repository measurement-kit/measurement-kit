// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/ooni.hpp>

#include <event2/dns.h>

namespace mk {
namespace ooni {

using namespace mk::report;

void dns_injection(std::string input, Settings options, Callback<Var<Entry>> cb,
                   Var<Reactor> reactor, Var<Logger> logger) {
    // Use libevent DNS engine for which we can force a specific nameserver.
    options["dns/engine"] = "libevent";
    // Force timeout according to dns_injection specification.
    options["dns/timeout"] = 3.0;
    Var<Entry> entry(new Entry);
    (*entry)["injected"] = nullptr;
    templates::dns_query(entry, "A", "IN", input, options["backend"],
                         [=](Error err, Var<dns::Message> message) {
                             logger->debug("dns_injection: got response");
                             if (!err && message->error_code == DNS_ERR_NONE) {
                                (*entry)["injected"] = true;
                             } else {
                                (*entry)["injected"] = false;
                             }
                             cb(entry);
                         },
                         options, reactor, logger);
}

} // namespace ooni
} // namespace mk
