// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef SRC_OONI_DNS_INJECTION_HPP
#define SRC_OONI_DNS_INJECTION_HPP

#include <event2/dns.h>
#include <measurement_kit/dns.hpp>
#include "src/ooni/ooni_test_impl.hpp"

namespace mk {
namespace ooni {

class DNSInjectionImpl : public OoniTestImpl {
    using OoniTestImpl::OoniTestImpl;

  public:
    DNSInjectionImpl(std::string input_filepath_, Settings options_)
        : OoniTestImpl(input_filepath_, options_) {
        test_name = "dns_injection";
        test_version = "0.0.1";
        needs_input = true;
    }

    void main(std::string input, Settings options,
              std::function<void(report::Entry)> cb) {
        dns_injection(input, options, [=](Var<Entry> entry) {
            cb(*entry);
        }, reactor, logger);
    }
};

} // namespace ooni
} // namespace mk
#endif
