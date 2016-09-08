// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/http.hpp>
#include <measurement_kit/ooni.hpp>
#include <regex>

namespace mk {
namespace ooni {
namespace resources {

using namespace mk::http;

template <MK_MOCK_NAMESPACE(http, get)>
void get_latest_release_url_impl(Callback<Error, std::string> callback,
                                 Settings settings, Var<Reactor> reactor,
                                 Var<Logger> logger) {
    std::string url =
        "https://github.com/OpenObservatory/ooni-resources/releases/latest";
    http_get(url, [=](Error error, Var<Response> response) {
        if (error) {
            callback(error, "");
            return;
        }
        if (response->status_code / 100 != 3) {
            callback(GenericError() /* XXX */, "");
            return;
        }
        if (response->headers.find("location") == response->headers.end()) {
            callback(GenericError() /* XXX */, "");
            return;
        }
        callback(NoError(), std::regex_replace(
            response->headers["location"],
            std::regex{
              "^https://github.com/OpenObservatory/ooni-resources/releases/tag/"
            },
            ""
        ));
    }, {}, settings, reactor, logger, nullptr, 0);
}

} // namespace resources
} // namespace mk
} // namespace ooni
