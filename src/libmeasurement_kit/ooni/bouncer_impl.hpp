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
 public:
    json response;

    static ErrorOr<Var<BouncerReply>> create(std::string, Var<Logger>);

    ErrorOr<std::string> get_collector();
    ErrorOr<std::string> get_collector_alternate(std::string type);
    ErrorOr<std::string> get_name();
    ErrorOr<std::string> get_test_helper(std::string name);
    ErrorOr<std::string> get_test_helper_alternate(std::string name, std::string type);
    ErrorOr<std::string> get_version();
 private:
    json get_base();
};

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
            }
            cb(reply.as_error(), nullptr);
        },
        reactor, logger, nullptr, 0);
}

} // namespace mk
} // namespace ooni
} // namespace bouncer
