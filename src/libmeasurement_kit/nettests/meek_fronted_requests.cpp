// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/nettests/runnable.hpp"
#include "src/libmeasurement_kit/ooni/nettests.hpp"

namespace mk {
namespace nettests {

MeekFrontedRequestsRunnable::MeekFrontedRequestsRunnable() noexcept {
    test_name = "meek_fronted_requests";
    test_version = "0.0.1";
    needs_input = true;
}

void MeekFrontedRequestsRunnable::main(std::string input, Settings options,
                                Callback<SharedPtr<nlohmann::json>> cb) {
    ooni::meek_fronted_requests(input, options, cb, reactor, logger);
}

} // namespace nettests
} // namespace mk
