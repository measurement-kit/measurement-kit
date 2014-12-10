#ifndef LIBIGHT_OONI_DNS_INJECTION_HPP
# define LIBIGHT_OONI_DNS_INJECTION_HPP

#include "protocols/dns.hpp"
#include "ooni/nettest.hpp"

namespace ight {
namespace ooni {
namespace dns_injection {


class DNSInjection : public nettest::NetTest {

    protocols::dns::Resolver resolver;

public:
    void setup(std::string input, ight::common::Settings options);

    void main(std::string input, ight::common::Settings options,
              std::function<void(ReportEntry)>&& cb);
};

}}}

#endif  // LIBIGHT_OONI_DNS_INJECTION_HPP
