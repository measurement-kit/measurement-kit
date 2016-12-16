// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../ooni/bouncer_impl.hpp"

namespace mk {
namespace ooni {
namespace bouncer {

ErrorOr<Var<BouncerReply>> BouncerReply::create(std::string data,
                                                Var<Logger> logger) {
    Var<BouncerReply> reply(new BouncerReply);
    try {
        reply->response = nlohmann::json::parse(data);
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

nlohmann::json BouncerReply::get_base() {
    // Note: this method can throw exceptions
    return response["net-tests"][0];
}

ErrorOr<std::string> BouncerReply::get_collector() {
    try {
        return get_base()["collector"];
    } catch (const std::exception &) {
        /* FALLTHROUGH */;
    }
    return BouncerValueNotFoundError();
}

ErrorOr<std::string> BouncerReply::get_collector_alternate(std::string type) {
    try {
        auto collectors = get_base()["collector-alternate"];
        for (auto collector : collectors) {
            if (collector["type"] == type) {
                return collector["address"];
            }
        }
    } catch (const std::exception &) {
        /* FALLTHROUGH */;
    }
    return BouncerValueNotFoundError();
}

ErrorOr<std::string> BouncerReply::get_name() {
    try {
        return get_base()["name"];
    } catch (const std::exception &) {
        /* FALLTHROUGH */;
    }
    return BouncerValueNotFoundError();
}

ErrorOr<std::string> BouncerReply::get_test_helper(std::string name) {
    try {
        return get_base()["test-helpers"][name];
    } catch (const std::exception &) {
        /* FALLTHROUGH */;
    }
    return BouncerValueNotFoundError();
}

ErrorOr<std::string> BouncerReply::get_test_helper_alternate(std::string name,
                                                             std::string type) {
    try {
        auto collectors = get_base()["test-helpers-alternate"][name];
        for (auto collector : collectors) {
            if (collector["type"] == type) {
                return collector["address"];
            }
        }
    } catch (const std::exception &) {
        /* FALLTHROUGH */;
    }
    return BouncerValueNotFoundError();
}

ErrorOr<std::string> BouncerReply::get_version() {
    try {
        return get_base()["version"];
    } catch (const std::exception &) {
        /* FALLTHROUGH */;
    }
    return BouncerValueNotFoundError();
}

void post_net_tests(std::string base_bouncer_url, std::string test_name,
                    std::string test_version, std::list<std::string> helpers,
                    Callback<Error, Var<BouncerReply>> cb, Settings settings,
                    Var<Reactor> reactor, Var<Logger> logger) {
    post_net_tests_impl(base_bouncer_url, test_name, test_version, helpers, cb,
                        settings, reactor, logger);
}

} // namespace mk
} // namespace ooni
} // namespace bouncer
