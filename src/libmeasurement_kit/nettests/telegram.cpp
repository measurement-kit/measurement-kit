// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/nettests/runnable.hpp"
#include "src/libmeasurement_kit/ooni/nettests.hpp"

namespace mk {
namespace nettests {

TelegramRunnable::TelegramRunnable() noexcept {
    test_name = "telegram";
    test_version = "0.0.2";
    needs_input = false;
}

void TelegramRunnable::main(std::string /*input*/, Settings options,
                            Callback<SharedPtr<report::Entry>> cb) {
    ooni::telegram(options, cb, reactor, logger);
}

} // namespace nettests
} // namespace mk
