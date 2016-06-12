// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <event2/dns.h>
#include <measurement_kit/dns.hpp>
#include <measurement_kit/ooni.hpp>
#include <measurement_kit/report.hpp>
#include <sstream>

namespace mk {
namespace ooni {

using namespace mk::report;

void dns_injection(std::string input, Settings options, Callback<Var<Entry>> cb,
                   Var<Reactor> reactor, Var<Logger> logger) {
    Var<Entry> entry(new Entry);
    (*entry)["injected"] = nullptr;
    templates::dns_query(entry, "A", "IN", input, options["backend"],
                         [=](dns::Message message) {
                             logger->debug("dns_injection: got response");
                             if (message.error_code == DNS_ERR_NONE) {
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
