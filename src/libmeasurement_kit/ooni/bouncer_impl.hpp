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

class BouncerReply {
 private:
     Var<Logger> logger;

 public:
    json response;

    static ErrorOr<Var<BouncerReply>> create(std::string, Var<Logger>);

    std::string get_collector();
    std::string get_collector_alternate(std::string type);
    std::string get_name();
    std::string get_test_helper(std::string name);
    std::string get_test_helper_alternate(std::string name, std::string type);
    std::string get_version();
};

ErrorOr<Var<BouncerReply>> BouncerReply::create(std::string data, Var<Logger> logger) {
    Var<BouncerReply> reply(new BouncerReply);
    try {
        reply->response = json::parse(data);
        if (reply->response.find("error") != reply->response.end()) {
            if (reply->response["error"] == "collector-not-found") {
                logger->warn(
                        "collector not found for the requested test");
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

std::string BouncerReply::get_collector() {
    try {
        return response["net-tests"][0]["collector"];
    } catch (std::domain_error &) {
        /* suppress */
    }
    return "";
}

std::string BouncerReply::get_collector_alternate(std::string type) {
    try {
        auto collectors = response["net-tests"][0]["collector-alternate"];
        for (auto collector : collectors) {
            if (collector["type"] == type)
                return collector["address"];
        }
    } catch (std::domain_error &) {
        /* suppress */
    }
    return "";
}

std::string BouncerReply::get_name() {
    try {
        return response["net-tests"][0]["name"];
    } catch (std::domain_error &) {
        /* suppress */
    }
    return "";
}

std::string BouncerReply::get_test_helper(std::string name) {
    try {
        return response["net-tests"][0]["test-helpers"][name];
    } catch (std::domain_error &) {
        /* suppress */
    }
    return "";
}

std::string BouncerReply::get_test_helper_alternate(std::string name, std::string type) {
    try {
        auto collectors = response["net-tests"][0]["test-helpers-alternate"][name];
        for (auto collector : collectors) {
            if (collector["type"] == type)
                return collector["address"];
        }
    } catch (std::domain_error &) {
        /* suppress */
    }
    return "";
}


std::string BouncerReply::get_version() {
    try {
        return response["net-tests"][0]["version"];
    } catch (std::domain_error &) {
        /* suppress */
    }
    return "";
}

template <MK_MOCK_NAMESPACE(http, request)>
void post_net_tests_impl(std::string base_bouncer_url, std::string test_name,
                    std::string test_version, std::list<std::string> helpers,
                    Callback<Error, Var<BouncerReply>> cb, Settings settings,
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
                cb(e, nullptr);
                return;
            }

            ErrorOr<Var<BouncerReply>> reply(BouncerReply::create(resp->body, logger));
            if (!!reply) {
                cb(NoError(), *reply);
                return;
            } else {
                cb(reply.as_error(), nullptr);
                return;
            }
        },
        reactor, logger, nullptr, 0);
}

} // namespace mk
} // namespace ooni
} // namespace bouncer
