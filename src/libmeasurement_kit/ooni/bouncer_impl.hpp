// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_OONI_BOUNCER_IMPL_HPP
#define SRC_LIBMEASUREMENT_KIT_OONI_BOUNCER_IMPL_HPP

#include <measurement_kit/ooni.hpp>

namespace mk {
namespace ooni {
namespace bouncer {

static inline nlohmann::json json_parse(std::string s) {
    return nlohmann::json::parse(s);
}

template <MK_MOCK(json_parse)>
ErrorOr<Var<BouncerReply>> create_impl(std::string data, Var<Logger> logger) {
    Var<BouncerReply> reply{new BouncerReply};
    try {
        reply->response = json_parse(data);
        if (reply->response.find("error") != reply->response.end()) {
            if (reply->response["error"] == "collector-not-found") {
                logger->warn("collector not found for the requested test");
                return BouncerCollectorNotFoundError();
            }
            if (reply->response["error"] == "invalid-request") {
                logger->warn("invalid request sent to the bouncer");
                return BouncerInvalidRequestError();
            }
            // I assume that if we receive a json with the key "error"
            // then the json has an unknown schema and we should not
            // continue to process it
            logger->warn("bouncer generic error");
            return BouncerGenericError();
        }
        if (reply->response["net-tests"].empty()) {
            logger->warn("generic bouncer error");
            return BouncerTestHelperNotFoundError();
        }
    } catch (std::invalid_argument &) {
        return JsonParseError();
    } catch (std::out_of_range &) {
        return JsonKeyError();
    } catch (std::domain_error &) {
        return JsonDomainError();
    }
    return reply;
}

template <MK_MOCK_AS(http::request, http_request)>
void post_net_tests_impl(std::string base_bouncer_url, std::string test_name,
                         std::string test_version,
                         std::list<std::string> helpers,
                         Callback<Error, Var<BouncerReply>> cb,
                         Settings settings, Var<Reactor> reactor,
                         Var<Logger> logger) {

    nlohmann::json request;
    nlohmann::json net_tests;
    net_tests["input-hashes"] = {};
    net_tests["name"] = test_name;
    net_tests["test-helpers"] = helpers;
    net_tests["version"] = test_version;
    request["net-tests"][0] = net_tests;

    std::string bbu = base_bouncer_url + "/bouncer/net-tests";
    std::string bm = "POST";
    settings["http/url"] = bbu;
    settings["http/method"] = bm;

    logger->debug("%sing to '%s' data '%s'", bm.c_str(), bbu.c_str(),
                  request.dump().c_str());
    http_request(settings, {{"Content-Type", "application/json"}},
                 request.dump(),
                 [=](Error error, Var<http::Response> resp) {
                     if (error) {
                         logger->warn("Bouncer error: %s",
                                      error.explain().c_str());
                         cb(error, nullptr);
                         return;
                     }

                     logger->debug("Bouncer reply: %s", resp->body.c_str());

                     ErrorOr<Var<BouncerReply>> reply(
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
