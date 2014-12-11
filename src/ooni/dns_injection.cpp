#include "ooni/dns_injection.hpp"

using namespace ight::ooni::dns_injection;

void
DNSInjection::main(std::string input, ight::common::Settings options,
                   std::function<void(ReportEntry)>&& cb)
{
    entry["injected"] = NULL;
    auto r = query(QueryType::A, QueryClass::IN,
                   input, options["nameserver"], [=](
                              protocols::dns::Response&& response) {
        ight_debug("Got response to DNS Injection test");
        if (response.get_evdns_status() == DNS_ERR_NONE) {
            entry["injected"] = true;
        } else {
            entry["injected"] = false;
        }
        cb(entry);
    });
}
