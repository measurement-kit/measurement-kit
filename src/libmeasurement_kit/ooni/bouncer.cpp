// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../ooni/bouncer_impl.hpp"

namespace mk {
namespace ooni {

ErrorOr<Var<BouncerReply>> BouncerReply::create(std::string data,
                                                Var<Logger> logger) {
    return bouncer::create_impl(data, logger);
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

namespace bouncer {

void post_net_tests(std::string base_bouncer_url, std::string test_name,
                    std::string test_version, std::list<std::string> helpers,
                    Callback<Error, Var<BouncerReply>> cb, Settings settings,
                    Var<Reactor> reactor, Var<Logger> logger) {
    post_net_tests_impl(base_bouncer_url, test_name, test_version, helpers, cb,
                        settings, reactor, logger);
}

} // namespace bouncer
} // namespace ooni
} // namespace mk
