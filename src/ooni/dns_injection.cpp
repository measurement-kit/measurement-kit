#include "ooni/protocols/dns.hpp"
#include "ooni/dns_injection.hpp"

using namespace ight::ooni::dns_injection;

DNSInjection::setup(std::string input, ight::common::Settings options) {
  resolver = Resolver({
      {"nameserver", options["nameserver"]},
      {"attempts", "1"},
  });
}

DNSInjection::main(std::string input, ight::common::Settings options,
                   std::function<void(ReportEntry)>&& func)
{
    auto entry = ReportEntry(input);
    entry["injected"] = NULL;
    auto r = resolver.request("A", input, [&](
                              Response&& response) {
        if (response.get_evdns_status() == DNS_ERR_NONE) {
            //entry[] = response.get_rtt();
            // XXX add support for setting the queries and answers for the DNS
            // base template.
            entry["injected"] = true;
        } else {
            entry["injected"] = false;
        }
    });

}
