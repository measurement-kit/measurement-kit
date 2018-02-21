// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_OONI_BOUNCER_IMPL_HPP
#define SRC_LIBMEASUREMENT_KIT_OONI_BOUNCER_IMPL_HPP

#include <measurement_kit/ooni/error.hpp>
#include <measurement_kit/common/json.hpp>

#include "src/libmeasurement_kit/common/mock.hpp"
#include "src/libmeasurement_kit/ooni/bouncer.hpp"
#include "src/libmeasurement_kit/http/http.hpp"

namespace mk {
namespace ooni {
namespace bouncer {

static Error
my_json_parse_process_and_filter_errors(const std::string &data,
                                        Callback<Json &> &&callable) {
    return json_process(data, std::move(callable));
}

template <MK_MOCK(my_json_parse_process_and_filter_errors)>
ErrorOr<SharedPtr<BouncerReply>> create_impl(std::string data, SharedPtr<Logger> logger) {
    SharedPtr<BouncerReply> reply{new BouncerReply};
    Error err = my_json_parse_process_and_filter_errors(data, [&](auto j) {
        reply->response = j;
        if (reply->response.find("error") != reply->response.end()) {
            if (reply->response["error"] == "collector-not-found") {
                throw BouncerCollectorNotFoundError();
            }
            if (reply->response["error"] == "invalid-request") {
                throw BouncerInvalidRequestError();
            }
            // I assume that if we receive a json with the key "error"
            // then the json has an unknown schema and we should not
            // continue to process it
            throw BouncerGenericError();
        }
        if (reply->response["net-tests"].empty()) {
            throw BouncerTestHelperNotFoundError();
        }
    });
    if (err) {
        logger->warn("bouncer parsing error: %s", err.what());
        return {err, {}};
    }
    return {NoError(), reply};
}

template <MK_MOCK_AS(http::request, http_request)>
void post_net_tests_impl(std::string base_bouncer_url, std::string test_name,
                         std::string test_version,
                         std::list<std::string> helpers,
                         Callback<Error, SharedPtr<BouncerReply>> cb,
                         Settings settings, SharedPtr<Reactor> reactor,
                         SharedPtr<Logger> logger) {

    Json request;
    Json net_tests;
    net_tests["input-hashes"] = {};
    net_tests["name"] = test_name;
    net_tests["test-helpers"] = helpers;
    net_tests["version"] = test_version;
    request["net-tests"][0] = net_tests;

    std::string bbu = base_bouncer_url + "/bouncer/net-tests";
    std::string bm = "POST";
    settings["http/url"] = bbu;
    settings["http/method"] = bm;

    http_request(settings, {{"Content-Type", "application/json"}},
                 request.dump(),
                 [=](Error error, SharedPtr<http::Response> resp) {
                     if (error) {
                         logger->warn("Bouncer error: %s", error.what());
                         cb(error, nullptr);
                         return;
                     }

                     ErrorOr<SharedPtr<BouncerReply>> reply(
                         BouncerReply::create(resp->body, logger));
                     if (!reply) {
                         cb(reply.as_error(), nullptr);
                         return;
                     }
                     cb(NoError(), *reply);
                 },
                 reactor, logger, nullptr, 0);
}

} // namespace bouncer
} // namespace ooni
} // namespace mk
#endif
