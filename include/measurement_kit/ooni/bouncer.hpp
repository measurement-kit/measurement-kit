// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_OONI_BOUNCER_HPP
#define MEASUREMENT_KIT_OONI_BOUNCER_HPP

#include <measurement_kit/common.hpp>
#include <measurement_kit/ext/json.hpp>

namespace mk {
namespace ooni {

class BouncerReply {
  public:
    nlohmann::json response;

    static ErrorOr<Var<BouncerReply>> create(std::string, Var<Logger>);

    ErrorOr<std::string> get_collector();
    ErrorOr<std::string> get_collector_alternate(std::string type);
    ErrorOr<std::string> get_name();
    ErrorOr<std::string> get_test_helper(std::string name);
    ErrorOr<std::string> get_test_helper_alternate(std::string name,
                                                   std::string type);
    ErrorOr<std::string> get_version();

  private:
    nlohmann::json get_base();
};

namespace bouncer {

void post_net_tests(std::string base_bouncer_url, std::string test_name,
                    std::string test_version, std::list<std::string> helpers,
                    Callback<Error, Var<BouncerReply>> cb, Settings settings,
                    Var<Reactor> reactor, Var<Logger> logger);

} // namespace bouncer
} // namespace ooni
} // namespace mk
#endif
