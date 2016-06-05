// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef SRC_OONI_DNS_INJECTION_HPP
#define SRC_OONI_DNS_INJECTION_HPP

#include <event2/dns.h>
#include <measurement_kit/dns.hpp>
#include "src/ooni/ooni_test_impl.hpp"
#include "src/ooni/dns_test_impl.hpp"

namespace mk {
namespace ooni {

using namespace mk::report;

class DNSInjectionImpl : public OoniTestImpl {
  public:
    DNSInjectionImpl(std::string input_filepath_, Settings options_)
            : OoniTestImpl(input_filepath_, options_) {
        test_name = "dns_injection";
        test_version = "0.0.1";

        validate_input_filepath();
    };

    void main(std::string input, Settings options,
              std::function<void(report::Entry)> &&cb) {
        dns_injection(options["backend"], input,
                      [this, cb](Error, Var<report::Entry> entry) {
                           cb(*entry);
                       }, options, reactor, logger);
    }
};

void dns_injection(std::string name_server, std::string input,
                   Callback<Error, Var<Entry>> callback, Settings options,
                   Var<Reactor> reactor, Var<Logger> logger) {
        Var<Entry> entry(new Entry);
        (*entry)["injected"] = nullptr;
        dns_template(entry, name_server, "A", "IN", input,
              [=](Error error, dns::Message message) {
                  logger->debug("dns_injection: got response");
                  if (message.error_code == DNS_ERR_NONE) {
                      (*entry)["injected"] = true;
                  } else {
                      (*entry)["injected"] = false;
                  }
                  callback(error, entry);
              }, options, reactor, logger);
    }

} // namespace ooni
} // namespace mk
#endif
