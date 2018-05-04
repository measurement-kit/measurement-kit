// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/ooni/bouncer_impl.hpp"

namespace mk {
namespace ooni {

namespace bouncer {
Error
my_json_parse_process_and_filter_errors(const std::string &data,
                                        Callback<Json &> &&callable) {
    return json_process(data, std::move(callable));
}
} // namespace bouncer

ErrorOr<SharedPtr<BouncerReply>> BouncerReply::create(std::string data,
                                                SharedPtr<Logger> logger) {
    return bouncer::create_impl(data, logger);
}

Json BouncerReply::get_base() {
    // Note: this method can throw exceptions
    return response["net-tests"][0];
}

ErrorOr<std::string> BouncerReply::get_collector() {
    try {
        return {NoError(), get_base()["collector"].get<std::string>()};
    } catch (const std::exception &) {
        /* FALLTHROUGH */;
    }
    return {BouncerValueNotFoundError(), std::string{}};
}

ErrorOr<std::string> BouncerReply::get_collector_alternate(std::string type) {
    try {
        auto collectors = get_base()["collector-alternate"];
        for (auto collector : collectors) {
            if (collector["type"] == type) {
                return {NoError(), collector["address"].get<std::string>()};
            }
        }
    } catch (const std::exception &) {
        /* FALLTHROUGH */;
    }
    return {BouncerValueNotFoundError(), std::string{}};
}

ErrorOr<std::string> BouncerReply::get_name() {
    try {
        return {NoError(), get_base()["name"].get<std::string>()};
    } catch (const std::exception &) {
        /* FALLTHROUGH */;
    }
    return {BouncerValueNotFoundError(), std::string{}};
}

ErrorOr<std::string> BouncerReply::get_test_helper(std::string name) {
    try {
        return {NoError(), get_base()["test-helpers"][name].get<std::string>()};
    } catch (const std::exception &) {
        /* FALLTHROUGH */;
    }
    return {BouncerValueNotFoundError(), std::string{}};
}

ErrorOr<std::string> BouncerReply::get_test_helper_alternate(std::string name,
                                                             std::string type) {
    try {
        auto collectors = get_base()["test-helpers-alternate"][name];
        for (auto collector : collectors) {
            if (collector["type"] == type) {
                return {NoError(), collector["address"].get<std::string>()};
            }
        }
    } catch (const std::exception &) {
        /* FALLTHROUGH */;
    }
    return {BouncerValueNotFoundError(), std::string{}};
}

ErrorOr<std::string> BouncerReply::get_version() {
    try {
        return {NoError(), get_base()["version"].get<std::string>()};
    } catch (const std::exception &) {
        /* FALLTHROUGH */;
    }
    return {BouncerValueNotFoundError(), std::string{}};
}

namespace bouncer {

void post_net_tests(std::string base_bouncer_url, std::string test_name,
                    std::string test_version, std::list<std::string> helpers,
                    Callback<Error, SharedPtr<BouncerReply>> cb, Settings settings,
                    SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    post_net_tests_impl(base_bouncer_url, test_name, test_version, helpers, cb,
                        settings, reactor, logger);
}

std::string production_bouncer_url() {
    return MK_OONI_PRODUCTION_BOUNCER_URL;
}

std::string testing_bouncer_url() {
    return MK_OONI_TESTING_BOUNCER_URL;
}

} // namespace bouncer
} // namespace ooni
} // namespace mk
