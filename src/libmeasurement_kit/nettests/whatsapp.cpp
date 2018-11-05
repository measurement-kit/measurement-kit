// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/nettests/runnable.hpp"
#include "src/libmeasurement_kit/ooni/nettests.hpp"

namespace mk {
namespace nettests {

WhatsappRunnable::WhatsappRunnable() noexcept {
    test_name = "whatsapp";
    test_version = "0.6.1";
    needs_input = false;
}

void WhatsappRunnable::main(std::string /*input*/, Settings options,
                            Callback<SharedPtr<nlohmann::json>> cb) {
    ooni::whatsapp(options, cb, reactor, logger);
}

} // namespace nettests
} // namespace mk
