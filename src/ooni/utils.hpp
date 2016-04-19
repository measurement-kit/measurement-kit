// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef SRC_OONI_UTILS_HPP
#define SRC_OONI_UTILS_HPP

#include <measurement_kit/common.hpp>
#include <measurement_kit/http.hpp>
#include <regex>
#include <string>

namespace mk {
namespace ooni {

template <decltype(http::get) http::get = http::get>
inline void ip_lookup(Callback<std::string> callback) {
    http::get("http://geoip.ubuntu.com/lookup",
              [=](Error err, http::Response response) {
                  if (err) {
                      callback(err, "");
                      return;
                  }
                  if (response.status_code != 200) {
                      callback(GenericError(), "");
                      return;
                  }
                  std::smatch m;
                  std::regex regex("<Ip>(.*)</Ip>");
                  if (std::regex_search(response.body, m, regex) == false) {
                      callback(GenericError(), "");
                      return;
                  }
                  callback(NoError(), m[1]);
              });
}

} // namespace ooni
} // namespace mk
#endif
