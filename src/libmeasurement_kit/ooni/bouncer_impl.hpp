// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_OONI_BOUNCER_IMPL_HPP
#define SRC_LIBMEASUREMENT_KIT_OONI_BOUNCER_IMPL_HPP

#include <measurement_kit/ooni.hpp>

namespace mk {
namespace ooni {
namespace bouncer {

template <MK_MOCK_NAMESPACE(http, request)>
void post_net_tests_impl(std::string base_bouncer_url, std::string test_name,
                    std::string test_version, std::list<std::string> helpers,
                    Callback<Error, Var<BouncerReply>> cb, Settings settings,
                    Var<Reactor> reactor, Var<Logger> logger) {

    nlohmann::json request;
    nlohmann::json net_tests;
    net_tests["input-hashes"] = {};
    net_tests["name"] = test_name;
    net_tests["test-helpers"] = helpers;
    net_tests["version"] = test_version;
    request["net-tests"][0] = net_tests;

    settings["http/url"] = base_bouncer_url;
    settings["http/method"] = "POST";
    http_request(
        settings, {{"Content-Type", "application/json"}}, request.dump(),
        [=](Error error, Var<http::Response> resp) {
            if (error) {
                cb(error, nullptr);
                return;
            }

            ErrorOr<Var<BouncerReply>> reply(BouncerReply::create(resp->body, logger));
            if (!reply) {
                cb(reply.as_error(), nullptr);
                return;
            }
            cb(NoError(), *reply);
        },
        reactor, logger, nullptr, 0);
}

} // namespace mk
} // namespace ooni
} // namespace bouncer
#endif
