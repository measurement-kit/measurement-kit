// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_OONI_DNS_INJECTION_TEST_HPP
#define MEASUREMENT_KIT_OONI_DNS_INJECTION_TEST_HPP

#include <measurement_kit/common.hpp>
#include <measurement_kit/report.hpp>

namespace mk {
namespace ooni {

using namespace mk::report;

MK_DECLARE_TEST_DSL(DnsInjectionTest)

void dns_injection(std::string, Settings, Callback<Var<Entry>>,
                   Var<Reactor> = Reactor::global(),
                   Var<Logger> = Logger::global());

} // namespace ooni
} // namespace mk
#endif
