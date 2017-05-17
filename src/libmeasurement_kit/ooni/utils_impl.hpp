// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_OONI_UTILS_IMPL_HPP
#define SRC_LIBMEASUREMENT_KIT_OONI_UTILS_IMPL_HPP

#include "../ooni/utils.hpp"

using json = nlohmann::json;

namespace mk {
namespace ooni {

template <MK_MOCK_AS(http::get, http_get)>
void ip_lookup_impl(Callback<Error, std::string> callback, Settings settings = {},
               Var<Reactor> reactor = Reactor::global(),
               Var<Logger> logger = Logger::global()) {
    http_get("http://geoip.ubuntu.com/lookup",
            [=](Error err, Var<http::Response> response) {
                if (err) {
                    callback(err, "");
                    return;
                }
                if (response->status_code != 200) {
                    callback(HttpRequestError(), "");
                    return;
                }
                std::smatch m;
                std::regex regex("<Ip>(.*)</Ip>");
                if (std::regex_search(response->body, m, regex) == false) {
                    callback(RegexSearchError(), "");
                    return;
                }
                if (!net::is_ip_addr(m[1])) {
                    callback(ValueError(), "");
                    return;
                }
                callback(NoError(), m[1]);
            },
            {}, settings, reactor, logger, nullptr, 0);
}

template <MK_MOCK_AS(dns::query, dns_query)>
void resolver_lookup_impl(Callback<Error, std::string> callback,
                          Settings settings = {},
                          Var<Reactor> reactor = Reactor::global(),
                          Var<Logger> logger = Logger::global()) {
  dns_query("IN", "A", "whoami.akamai.net",
      [=](Error error, Var<dns::Message> message) {
        if (!error) {
          for (auto answer : message->answers) {
            if (answer.ipv4 != "") {
              logger->debug("ip address of resolver is %s",
                            answer.ipv4.c_str());
              callback(NoError(), answer.ipv4);
              return;
            }
          }
        } else {
          logger->debug("failed to lookup resolver ip address");
          callback(error, "");
          return;
        }
      }, settings, reactor, logger);
}

} // namespace ooni
} // namespace mk
#endif
