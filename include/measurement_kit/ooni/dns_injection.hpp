// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_OONI_DNS_INJECTION_HPP
#define MEASUREMENT_KIT_OONI_DNS_INJECTION_HPP

#include <measurement_kit/common.hpp>
#include <measurement_kit/ooni/ooni_test.hpp>
#include <measurement_kit/report.hpp>

namespace mk {
namespace ooni {

using namespace mk::report;

void dns_injection(std::string, Settings, Callback<Var<Entry>>,
                   Var<Reactor> = Reactor::global(),
                   Var<Logger> = Logger::global());

class DnsInjection : public OoniTest {
  public:
    DnsInjection() : DnsInjection("", {}) {}
    DnsInjection(std::string f, Settings o) : OoniTest(f, o) {
        test_name = "dns_injection";
        test_version = "0.0.1";
        needs_input = true;
    }

    void main(std::string input, Settings options,
              Callback<report::Entry> cb) override {
        dns_injection(input, options, [=](Var<Entry> entry) {
            cb(*entry);
        }, reactor, logger);
    }

    Var<NetTest> create_test_() override;
};

} // namespace ooni
} // namespace mk
#endif
