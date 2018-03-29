// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_OONI_BOUNCER_HPP
#define SRC_LIBMEASUREMENT_KIT_OONI_BOUNCER_HPP

#include <list>
#include <measurement_kit/common.hpp>

namespace mk {
namespace ooni {

class BouncerReply {
  public:
    Json response;

    static ErrorOr<SharedPtr<BouncerReply>> create(std::string, SharedPtr<Logger>);

    ErrorOr<std::string> get_collector();
    ErrorOr<std::string> get_collector_alternate(std::string type);
    ErrorOr<std::string> get_name();
    ErrorOr<std::string> get_test_helper(std::string name);
    ErrorOr<std::string> get_test_helper_alternate(std::string name,
                                                   std::string type);
    ErrorOr<std::string> get_version();

  private:
    Json get_base();
};

namespace bouncer {

void post_net_tests(std::string base_bouncer_url, std::string test_name,
                    std::string test_version, std::list<std::string> helpers,
                    Callback<Error, SharedPtr<BouncerReply>> cb, Settings settings,
                    SharedPtr<Reactor> reactor, SharedPtr<Logger> logger);

#define MK_OONI_PRODUCTION_BOUNCER_URL "https://bouncer.ooni.io"
#define MK_OONI_TESTING_BOUNCER_URL "https://bouncer.test.ooni.io"

std::string production_bouncer_url();
std::string testing_bouncer_url();

} // namespace bouncer
} // namespace ooni
} // namespace mk
#endif
