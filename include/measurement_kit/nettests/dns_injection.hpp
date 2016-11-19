// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_NETTESTS_OONI_DNS_INJECTION_HPP
#define MEASUREMENT_KIT_NETTESTS_OONI_DNS_INJECTION_HPP

#include <measurement_kit/ooni.hpp>
#include <measurement_kit/report.hpp>

namespace mk {
namespace nettests {

class DnsInjection : public ooni::OoniTest {
  public:
    DnsInjection() : DnsInjection("", {}) {}
    DnsInjection(std::string f, Settings o) : OoniTest(f, o) {
        test_name = "dns_injection";
        test_version = "0.0.1";
        needs_input = true;
    }

    void main(std::string input, Settings options,
              Callback<report::Entry> cb) override {
        ooni::dns_injection(input, options, [=](Var<report::Entry> entry) {
            cb(*entry);
        }, reactor, logger);
    }

    Var<NetTest> create_test_() override;
};

} // namespace nettests
} // namespace mk
#endif
