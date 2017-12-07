// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "private/nettests/runnable.hpp"
#include <measurement_kit/nettests.hpp>
#include <measurement_kit/ooni.hpp>

namespace mk {
namespace nettests {

WhatsappTest::WhatsappTest() : BaseTest() {
    runnable.reset(new WhatsappRunnable);
    runnable->test_name = "whatsapp";
    runnable->test_version = "0.6.1";
    runnable->needs_input = false;
}

void WhatsappRunnable::main(std::string /*input*/, Settings options,
                            Callback<SharedPtr<report::Entry>> cb) {
    ooni::whatsapp(options, cb, reactor, logger);
}

} // namespace nettests
} // namespace mk
