// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_OONI_TEMPLATES_IMPL_HPP
#define SRC_LIBMEASUREMENT_KIT_OONI_TEMPLATES_IMPL_HPP

#include <measurement_kit/ooni.hpp>
#include <measurement_kit/http.hpp>

#include <event2/dns.h>

#include "src/libmeasurement_kit/net/emitter.hpp"
#include "src/libmeasurement_kit/ooni/utils.hpp"

namespace mk {
namespace ooni {
namespace templates {

using namespace mk::report;

// Mockable implementation of OONI's http_request() template where we can
// override the underlying function we use in regress tests.
template <decltype(http::request) mocked_http_request>
void http_request_impl(SharedPtr<Entry> entry, Settings settings,
                       http::Headers headers, std::string body,
                       Callback<Error, SharedPtr<http::Response>> cb,
                       SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {

    (*entry)["agent"] = "agent";
    (*entry)["socksproxy"] = nullptr;

    // Include the name of the agent, like ooni-probe does
    ErrorOr<int> max_redirects = settings.get_noexcept("http/max_redirects", 0);
    if (!!max_redirects && *max_redirects > 0) {
        (*entry)["agent"] = "redirect";
    }

    if (settings.find("http/method") == settings.end()) {
        settings["http/method"] = "GET";
    }

    /*
     * XXX probe ip passed down the stack to allow us to scrub it from the
     * entry; see issue #1110 for plans to make this better.
     */
    std::string probe_ip = settings.get("real_probe_ip_", std::string{});
    auto redact = [=](std::string s) {
        if (probe_ip != "" && !settings.get("save_real_probe_ip", false)) {
            s = mk::ooni::scrub(s, probe_ip);
        }
        return s;
    };

    mocked_http_request(
        settings, headers, body,
        [=](Error error, SharedPtr<http::Response> response) {

            auto dump = [&](SharedPtr<http::Response> response) {
                Entry rr;

                if (!!error) {
                    rr["failure"] = error.reason;
                } else {
                    rr["failure"] = nullptr;
                }

                /*
                 * Note: we should not assume that, if the response is set,
                 * then also the request will be set. The response should
                 * be allocated in all cases because that's what is returned
                 * by the callback, while the request may not be allocated
                 * when we fail before filling a response (i.e. when we
                 * cannot connect). For sure, the HTTP code should be made
                 * less unpredictable, but that's not a good excuse for not
                 * performing sanity checks also at this level.
                 *
                 * See <measurement-kit/measurement-kit#1169>.
                 */
                if (!!response && !!response->request) {
                    /*
                     * Note: `probe_ip` comes from an external service, hence
                     * we MUST call `represent_string` _after_ `redact()`.
                     */
                    for (auto pair : response->headers) {
                        rr["response"]["headers"][pair.first] =
                            represent_string(redact(pair.second));
                    }
                    rr["response"]["body"] =
                        represent_string(redact(response->body));
                    rr["response"]["response_line"] =
                        represent_string(redact(response->response_line));
                    rr["response"]["code"] = response->status_code;

                    auto request = response->request;
                    // Note: we checked above that we can deref `request`
                    for (auto pair : request->headers) {
                        rr["request"]["headers"][pair.first] =
                            represent_string(redact(pair.second));
                    }
                    rr["request"]["body"] =
                        represent_string(redact(request->body));
                    rr["request"]["url"] = request->url.str();
                    rr["request"]["method"] = request->method;
                    rr["request"]["tor"] = {{
                        "exit_ip", nullptr
                    }, {
                        "exit_name", nullptr
                    }, {
                        "is_tor", false
                    }};
                }
                return rr;
            };

            if (!!response) {
                for (SharedPtr<http::Response> x = response; !!x; x = x->previous) {
                    (*entry)["requests"].push_back(dump(x));
                }
            } else {
                (*entry)["requests"].push_back(dump(response));
            }
            cb(error, response);
        },
        reactor, logger, {}, 0);
}

} // namespace templates
} // namespace ooni
} // namespace mk
#endif
