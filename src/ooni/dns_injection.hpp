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
    DNSInjection(std::string input_filepath_, ight::common::Settings options_) : 
      DNSTest(input_filepath_, options_) {
        test_name = "dns_injection";
        test_version = "0.0.1";
    };

    void main(std::string input, ight::common::Settings options,
              std::function<void(ReportEntry)>&& cb);
};

}}}

#endif  // LIBIGHT_OONI_DNS_INJECTION_HPP
