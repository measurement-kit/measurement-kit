// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_OONI_NETTESTS_HPP
#define MEASUREMENT_KIT_OONI_NETTESTS_HPP

#include <measurement_kit/report.hpp>

namespace mk {
namespace ooni {

void captiveportal(std::string, Settings, Callback<SharedPtr<report::Entry>>,
                    SharedPtr<Reactor> = Reactor::global(),
                    SharedPtr<Logger> = Logger::global());

void dns_injection(std::string, Settings, Callback<SharedPtr<report::Entry>>,
                   SharedPtr<Reactor> = Reactor::global(),
                   SharedPtr<Logger> = Logger::global());

void http_invalid_request_line(Settings, Callback<SharedPtr<report::Entry>>,
                               SharedPtr<Reactor> = Reactor::global(),
                               SharedPtr<Logger> = Logger::global());

void tcp_connect(std::string, Settings, Callback<SharedPtr<report::Entry>>,
                 SharedPtr<Reactor> = Reactor::global(),
                 SharedPtr<Logger> = Logger::global());

void web_connectivity(std::string input, Settings,
                      Callback<SharedPtr<report::Entry>>,
                      SharedPtr<Reactor> = Reactor::global(),
                      SharedPtr<Logger> = Logger::global());

void meek_fronted_requests(std::string input, Settings,
                           Callback<SharedPtr<report::Entry>>,
                           SharedPtr<Reactor> = Reactor::global(),
                           SharedPtr<Logger> = Logger::global());

void http_header_field_manipulation(std::string input, Settings,
                                    Callback<SharedPtr<report::Entry>>,
                                    SharedPtr<Reactor> = Reactor::global(),
                                    SharedPtr<Logger> = Logger::global());

void telegram(Settings, Callback<SharedPtr<report::Entry>>,
              SharedPtr<Reactor> = Reactor::global(),
              SharedPtr<Logger> = Logger::global());
  
void facebook_messenger(Settings, Callback<SharedPtr<report::Entry>>,
                        SharedPtr<Reactor> = Reactor::global(),
                        SharedPtr<Logger> = Logger::global());

void whatsapp(Settings, Callback<Var<report::Entry>>,
              Var<Reactor> = Reactor::global(),
              Var<Logger> = Logger::global());

} // namespace ooni
} // namespace mk
#endif
