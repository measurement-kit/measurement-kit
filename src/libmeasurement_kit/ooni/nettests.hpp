// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_OONI_NETTESTS_HPP
#define SRC_LIBMEASUREMENT_KIT_OONI_NETTESTS_HPP

#include "src/libmeasurement_kit/common/settings.hpp"
#include "src/libmeasurement_kit/common/reactor.hpp"

namespace mk {
namespace ooni {

void captiveportal(std::string, Settings, Callback<SharedPtr<nlohmann::json>>,
                    SharedPtr<Reactor>, SharedPtr<Logger>);

void dns_injection(std::string, Settings, Callback<SharedPtr<nlohmann::json>>,
                   SharedPtr<Reactor>, SharedPtr<Logger>);

void http_invalid_request_line(Settings, Callback<SharedPtr<nlohmann::json>>,
                               SharedPtr<Reactor>, SharedPtr<Logger>);

void tcp_connect(std::string, Settings, Callback<SharedPtr<nlohmann::json>>,
                 SharedPtr<Reactor>, SharedPtr<Logger>);

void web_connectivity(std::string input, Settings,
                      Callback<SharedPtr<nlohmann::json>>,
                      SharedPtr<Reactor>, SharedPtr<Logger>);

void meek_fronted_requests(std::string input, Settings,
                           Callback<SharedPtr<nlohmann::json>>,
                           SharedPtr<Reactor>, SharedPtr<Logger>);

void http_header_field_manipulation(std::string input, Settings,
                                    Callback<SharedPtr<nlohmann::json>>,
                                    SharedPtr<Reactor>, SharedPtr<Logger>);

void telegram(Settings, Callback<SharedPtr<nlohmann::json>>,
              SharedPtr<Reactor>, SharedPtr<Logger>);
  
void facebook_messenger(Settings, Callback<SharedPtr<nlohmann::json>>,
                        SharedPtr<Reactor>, SharedPtr<Logger>);

void whatsapp(Settings, Callback<SharedPtr<nlohmann::json>>,
              SharedPtr<Reactor>, SharedPtr<Logger>);

} // namespace ooni
} // namespace mk
#endif
