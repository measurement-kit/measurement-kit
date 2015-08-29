// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/ooni/dns_injection.hpp>
#include <sys/stat.h>

namespace measurement_kit {
namespace ooni {

using namespace measurement_kit::common;
using namespace measurement_kit::dns;

void
DNSInjection::main(std::string input, Settings options,
                   std::function<void(ReportEntry)>&& cb)
{
    entry["injected"] = NULL;
    have_entry = cb;
    query(QueryType::A, QueryClass::IN,
                   input, options["nameserver"], [this](
                              Response&& response) {
        logger.debug("dns_injection: got response");
        if (response.get_evdns_status() == DNS_ERR_NONE) {
            entry["injected"] = true;
        } else {
            entry["injected"] = false;
        }
        have_entry(entry);
    });
}

}}
