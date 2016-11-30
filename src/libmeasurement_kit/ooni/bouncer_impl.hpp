// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/ext.hpp>
#include <measurement_kit/http.hpp>
#include <measurement_kit/ooni.hpp>

using json = nlohmann::json;

namespace mk {
namespace ooni {
namespace bouncer {

struct Reply {
    std::string https_collector;
    std::string https_helper;

    std::string cloudfront_collector;
    std::string cloudfront_helper;
};

template <MK_MOCK_NAMESPACE(http, request)>
void post_net_tests_impl(std::string base_bouncer_url, std::string test_name,
                    std::string test_version, std::list<std::string> helpers,
                    Callback<Error, Var<Reply>> cb, Settings settings,
                    Var<Reactor> reactor, Var<Logger> logger) {

    json request;
    json net_tests;
    net_tests["input-hashes"] = {};
    net_tests["name"] = test_name;
    net_tests["test-helpers"] = helpers;
    net_tests["version"] = test_version;
    request["net-tests"][0] = net_tests;

    settings["http/url"] = base_bouncer_url;
    settings["http/method"] = "POST";
    http_request(
        settings, {}, request.dump(),
        [=](Error e, Var<http::Response> resp) {
            if (e) {
                reactor->call_soon([=]() { cb(e, nullptr); });
                return;
            }
            Var<Reply> reply(new Reply);
            try {
                json response = json::parse(resp->body);
                if (response.find("error") != response.end()) {
                    if (response["error"] == "collector-not-found") {
                        logger->warn(
                            "collector not found for the requested test");
                        reactor->call_soon([=]() {
                            cb(BouncerCollectorNotFoundError(), nullptr);
                        });
                        return;
                    }
                    if (response["error"] == "invalid-request") {
                        logger->warn("invalid request sent to the bouncer");
                        reactor->call_soon([=]() {
                            cb(BouncerInvalidRequestError(), nullptr);
                        });
                        return;
                    }
                    // I assume that if we receive a json with the key "error"
                    // then the json has an unknown schema and we should not
                    // continue to process it
                    logger->warn("bouncer generic error");
                    reactor->call_soon(
                        [=]() { cb(BouncerGenericError(), nullptr); });
                    return;
                }
                if (response["net-tests"].empty()) {
                    logger->warn("generic bouncer error");
                    reactor->call_soon([=]() {
                            cb(BouncerTestHelperNotFoundError(), nullptr);
                            });
                    return;
                }
                auto collector_alternate =
                    response["net-tests"][0]["collector-alternate"];
                for (auto collector : collector_alternate) {
                    if (collector["type"] == "https") {
                        reply->https_collector = collector["address"];
                    } else if (collector["type"] == "cloudfront") {
                        reply->cloudfront_collector = collector["address"];
                    }
                }
                auto test_helpers_alternate =
                    response["net-tests"][0]["test-helpers-alternate"]
                            [test_name];
                for (auto helper : test_helpers_alternate) {
                    if (helper["type"] == "https") {
                        reply->https_helper = helper["address"];
                    } else if (helper["type"] == "cloudfront") {
                        reply->cloudfront_helper = helper["address"];
                    }
                }
            } catch (std::invalid_argument &) {
                cb(JsonParseError(), nullptr);
                return;
            } catch (std::out_of_range &) {
                cb(JsonKeyError(), nullptr);
                return;
            } catch (std::domain_error &) {
                cb(JsonDomainError(), nullptr);
                return;
            }
            reactor->call_soon([=]() { cb(NoError(), reply); });
        },
        reactor, logger, nullptr, 0);
};

} // namespace mk
} // namespace ooni
} // namespace bouncer
