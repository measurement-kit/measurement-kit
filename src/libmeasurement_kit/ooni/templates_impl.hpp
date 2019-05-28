// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_OONI_TEMPLATES_IMPL_HPP
#define SRC_LIBMEASUREMENT_KIT_OONI_TEMPLATES_IMPL_HPP

#include <event2/dns.h>

#include "src/libmeasurement_kit/ooni/error.hpp"
#include "src/libmeasurement_kit/http/http.hpp"
#include "src/libmeasurement_kit/net/emitter.hpp"
#include "src/libmeasurement_kit/ooni/utils.hpp"

namespace mk {
namespace ooni {
namespace templates {

// Mockable implementation of OONI's http_request() template where we can
// override the underlying function we use in regress tests.
template <decltype(http::request) mocked_http_request>
void http_request_impl(SharedPtr<nlohmann::json> entry, Settings settings,
                       http::Headers headers, std::string body,
                       Callback<Error, SharedPtr<http::Response>> cb,
                       SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {

    (*entry)["socksproxy"] = nullptr;

    // Include the name of the agent, like ooni-probe does
    (*entry)["agent"] = "agent";
    ErrorOr<int> max_redirects = settings.get_noexcept("http/max_redirects", 0);
    if (!!max_redirects && *max_redirects > 0) {
        (*entry)["agent"] = "redirect";
    }

    if (settings.find("http/method") == settings.end()) {
        settings["http/method"] = "GET";
    }

    mocked_http_request(
        settings, headers, body,
        [=](Error error, SharedPtr<http::Response> response) {

            auto dump = [&](SharedPtr<http::Response> response, bool first) {
                nlohmann::json rr;

                // We only set the error for the first response. This fixes
                // the bug documented in issue #1549.
                if (!!error && first) {
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
                    // TODO(bassosimone): we currently pass around a Response
                    // encapsulating a Request. However, after #1604 there are
                    // cases where there is a dummy Response encapsulating a
                    // Request (e.g. when connection is refused). Here we are
                    // using `response_line`: if that isn't set it mean we did
                    // not even receive any response byte. In such case, we
                    // format the output according to OONI specification. It
                    // can be improved by using more flexible typing (e.g.
                    // nhlomann::json to represent request and response).
                    if (response->response_line != "") {
                        /*
                        * Note: `probe_ip` comes from an external service, hence
                        * we MUST call `represent_string` _after_ `redact()`.
                        */
                        for (auto h : response->headers) {
                            rr["response"]["headers"][h.key] =
                                represent_string(redact(settings, h.value));
                        }
                        rr["response"]["body"] =
                            represent_string(redact(settings, response->body));
                        rr["response"]["response_line"] =
                            represent_string(redact(settings, response->response_line));
                        rr["response"]["code"] = response->status_code;
                    } else {
                        rr["response"]["body"] = nullptr;
                        rr["response"]["headers"] = nlohmann::json::object();
                    }
                    auto request = response->request;
                    // Note: we checked above that we can deref `request`
                    for (auto h : request->headers) {
                        rr["request"]["headers"][h.key] =
                            represent_string(redact(settings, h.value));
                    }
                    rr["request"]["body"] =
                        represent_string(redact(settings, request->body));
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
                bool first = true;
                for (SharedPtr<http::Response> x = response; !!x; x = x->previous) {
                    (*entry)["requests"].push_back(dump(x, first));
                    first = false;
                }
            } else {
                (*entry)["requests"].push_back(dump(response, true));
            }
            cb(error, response);
        },
        reactor, logger, {}, 0);
}

} // namespace templates
} // namespace ooni
} // namespace mk
#endif
