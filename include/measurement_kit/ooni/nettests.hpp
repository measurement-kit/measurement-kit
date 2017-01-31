// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_OONI_NETTESTS_HPP
#define MEASUREMENT_KIT_OONI_NETTESTS_HPP

#include <measurement_kit/report.hpp>

namespace mk {
namespace ooni {

void dns_injection(std::string, Settings, Callback<Var<report::Entry>>,
                   Var<Reactor> = Reactor::global(),
                   Var<Logger> = Logger::global());

void http_invalid_request_line(Settings, Callback<Var<report::Entry>>,
                               Var<Reactor> = Reactor::global(),
                               Var<Logger> = Logger::global());

void tcp_connect(std::string, Settings, Callback<Var<report::Entry>>,
                 Var<Reactor> = Reactor::global(),
                 Var<Logger> = Logger::global());

void web_connectivity(std::string input, Settings,
                      Callback<Var<report::Entry>>,
                      Var<Reactor> = Reactor::global(),
                      Var<Logger> = Logger::global());

void meek_fronting(Settings, Callback<Var<report::Entry>>,
                   Var<Reactor> = Reactor::global(),
                   Var<Logger> = Logger::global());


} // namespace ooni
} // namespace mk
#endif
