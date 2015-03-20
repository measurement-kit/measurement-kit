#include <ight/ooni/dns_injection.hpp>
#include <sys/stat.h>

using namespace ight::common::settings;
using namespace ight::ooni::dns_injection;

void
DNSInjection::main(std::string input, Settings options,
                   std::function<void(ReportEntry)>&& cb)
{
    entry["injected"] = NULL;
    have_entry = cb;
    query(QueryType::A, QueryClass::IN,
                   input, options["nameserver"], [this](
                              protocols::dns::Response&& response) {
        ight_debug("Got response to DNS Injection test");
        if (response.get_evdns_status() == DNS_ERR_NONE) {
            entry["injected"] = true;
        } else {
            entry["injected"] = false;
        }
        have_entry(entry);
    });
}
