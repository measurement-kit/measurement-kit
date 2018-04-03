// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/ooni/nettests.hpp"
#include "src/libmeasurement_kit/ooni/templates.hpp"
#include "src/libmeasurement_kit/report/entry.hpp"

#include <measurement_kit/ooni.hpp>

#include <event2/dns.h>

namespace mk {
namespace ooni {

using namespace mk::report;

void dns_injection(std::string input, Settings options, Callback<SharedPtr<Entry>> cb,
                   SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    // Use libevent DNS engine for which we can force a specific nameserver.
    options["dns/engine"] = "libevent";
    // Force timeout according to dns_injection specification.
    options["dns/timeout"] = 3.0;
    SharedPtr<Entry> entry(new Entry);
    (*entry)["injected"] = nullptr;
    templates::dns_query(entry, "A", "IN", input, options["backend"],
                         [=](Error err, SharedPtr<dns::Message> message) {
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
