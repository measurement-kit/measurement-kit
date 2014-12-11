#ifndef LIBIGHT_OONI_DNS_INJECTION_HPP
# define LIBIGHT_OONI_DNS_INJECTION_HPP

#include "protocols/dns.hpp"
#include "ooni/net_test.hpp"
#include "ooni/dns_test.hpp"

using namespace ight::ooni::dns_test;

namespace ight {
namespace ooni {
namespace dns_injection {

class DNSInjection : public DNSTest {
  using DNSTest::DNSTest;

public:
    void main(std::string input, ight::common::Settings options,
              std::function<void(ReportEntry)>&& cb);
};

}}}

#endif  // LIBIGHT_OONI_DNS_INJECTION_HPP
