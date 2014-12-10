#ifndef LIBIGHT_OONI_DNS_INJECTION_HPP
# define LIBIGHT_OONI_DNS_INJECTION_HPP

#include "ooni/nettest.hpp"

namespace ight {
namespace ooni {
namespace dns_injection {


class DNSInjection : public NetTest {

    Resolver resolver;

public:
    void main(ight::common::Settings options,
              std::function<void(ReportEntry)>&& func);

    void main(std::string input, ight::common::Settings options,
              std::function<void(ReportEntry)>&& func);
}

}}}

#endif  // LIBIGHT_OONI_DNS_INJECTION_HPP
