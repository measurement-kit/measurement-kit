// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_OONI_HTTP_TEST_HPP
#define MEASUREMENT_KIT_OONI_HTTP_TEST_HPP

#include <measurement_kit/common/settings.hpp>
#include <measurement_kit/http/client.hpp>
#include <measurement_kit/ooni/net_test.hpp>

namespace measurement_kit {
namespace ooni {

using namespace measurement_kit::common;
using namespace measurement_kit;

class HTTPTest : public ooni::NetTest {
    using ooni::NetTest::NetTest;

    http::Client http_client;

public:
    HTTPTest(std::string input_filepath_, Settings options_) :
      ooni::NetTest(input_filepath_, options_) {
        test_name = "tcp_test";
        test_version = "0.0.1";
    };

    HTTPTest(Settings options_) :
      HTTPTest("", options_)  {};


    void request(Settings settings, http::Headers headers,
                 std::string body,
                 http::RequestCallback&& callback) {

        http_client.request(settings, headers, body,
                            [=](Error error, http::Response&& response) {

            YAML::Node rr;
            rr["request"]["headers"] = std::map<std::string, std::string>(headers);
            rr["request"]["body"] = body;
            rr["request"]["url"] = settings.at("url");
            rr["request"]["http_version"] = settings.at("http_version");
            rr["request"]["method"] = settings.at("method");

            // XXX we should probably update the OONI data format to remove
            // this.
            rr["method"] = settings.at("method");

            if (error == 0) {
                rr["response"]["headers"] = std::map<std::string, std::string>(response.headers);
                rr["response"]["body"] = response.body.read();
                rr["response"]["response_line"] = response.response_line;
                rr["response"]["code"] = response.status_code;
            } else {
                rr["failure"] = "unknown_failure measurement_kit_error";
                rr["error_code"] = (int) error;
            }

            entry["requests"].push_back(rr);
            entry["agent"] = "agent";
            entry["socksproxy"] = "";
            callback(error, std::move(response));
        }, &logger);
    };

};

}}
#endif
