// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef SRC_OONI_HTTP_TEST_HPP
#define SRC_OONI_HTTP_TEST_HPP

#include <measurement_kit/common/settings.hpp>
#include <measurement_kit/http.hpp>
#include "src/ooni/ooni_test_impl.hpp"

using json = nlohmann::json;

namespace mk {
namespace ooni {

class HTTPTestImpl : public ooni::OoniTestImpl {
    using ooni::OoniTestImpl::OoniTestImpl;

  public:
    HTTPTestImpl(std::string input_filepath_, Settings options_)
        : ooni::OoniTestImpl(input_filepath_, options_) {
        test_name = "tcp_test";
        test_version = "0.0.1";
    }

    HTTPTestImpl(Settings options_) : HTTPTestImpl("", options_) {}

    void request(Settings settings, http::Headers headers, std::string body,
                 http::RequestCallback &&callback) {

        // XXX This has been refactored from using http::Client to using
        // the http::request() interface. The problem now is that the previous
        // semantic of client was that, when the client dies, it stops any
        // running request. The new semantic has not this feature, which
        // makes the code much simpler, but which is *not* consistent with
        // the assumptions made in this class, which could in theory be
        // destroyed while the request is running. To avoid this potential
        // memory error, we shall pass a smart pointer to this class to
        // the lambda closure rather than `this` implicitly.
        //
        // That said, I think that the current usage of this code is OK
        // but this comment's here to signal refactoring is needed.
        http::request(
            settings, headers,
            body, [=](Error error, http::Response response) {

                json rr;
                rr["request"]["headers"] =
                    std::map<std::string, std::string>(headers);
                rr["request"]["body"] = body;
                rr["request"]["url"] = settings.at("url").str();
                rr["request"]["method"] = settings.at("method").str();

                // XXX we should probably update the OONI data format to remove
                // this.
                rr["method"] = settings.at("method").str();

                if (error == 0) {
                    rr["response"]["headers"] =
                        std::map<std::string, std::string>(response.headers);
                    rr["response"]["body"] = response.body;
                    rr["response"]["response_line"] = response.response_line;
                    rr["response"]["code"] = response.status_code;
                } else {
                    rr["failure"] = "unknown_failure measurement_kit_error";
                    rr["error_code"] = (int)error;
                }

                entry["requests"].push_back(rr);
                entry["agent"] = "agent";
                entry["socksproxy"] = "";
                callback(error, std::move(response));
            }, logger, reactor);
    }
};

} // namespace ooni
} // namespace mk
#endif
