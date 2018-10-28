// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_OONI_UTILS_IMPL_HPP
#define SRC_LIBMEASUREMENT_KIT_OONI_UTILS_IMPL_HPP

#include "src/libmeasurement_kit/common/encoding.hpp"
#include "src/libmeasurement_kit/common/mock.hpp"
#include "../ooni/utils.hpp"

#include "src/libmeasurement_kit/http/http.hpp"

#include "src/libmeasurement_kit/regexp/regexp.hpp"

namespace mk {
namespace ooni {

template <MK_MOCK_AS(dns::query, dns_query)>
void resolver_lookup_impl(Callback<Error, std::string> callback,
                          Settings settings = {},
                          SharedPtr<Reactor> reactor = Reactor::global(),
                          SharedPtr<Logger> logger = Logger::global()) {
  dns_query("IN", "A", "whoami.akamai.net",
      [=](Error error, SharedPtr<dns::Message> message) {
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
