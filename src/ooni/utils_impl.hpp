// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_OONI_UTILS_IMPL_HPP
#define SRC_OONI_UTILS_IMPL_HPP

#include "src/ooni/utils.hpp"
#include <GeoIP.h>
#include <measurement_kit/common.hpp>
#include <measurement_kit/ext.hpp>
#include <measurement_kit/http.hpp>
#include <regex>
#include <string>

using json = nlohmann::json;

namespace mk {
namespace ooni {

template <MK_MOCK_NAMESPACE(http, get)>
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
                    callback(GenericError(), "");
                    return;
                }
                std::smatch m;
                std::regex regex("<Ip>(.*)</Ip>");
                if (std::regex_search(response->body, m, regex) == false) {
                    callback(GenericError(), "");
                    return;
                }
                callback(NoError(), m[1]);
            },
            {}, settings, reactor, logger);
}

} // namespace ooni
} // namespace mk
#endif
