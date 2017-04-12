// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../common/utils.hpp"
#include "../ooni/constants.hpp"
#include <measurement_kit/ooni.hpp>

namespace mk {
namespace ooni {

using namespace mk::report;

void telegram(std::string input, Settings options,
              Callback<Var<report::Entry>> callback,
              Var<Reactor> reactor, Var<Logger> logger) {
    logger->info("starting telegram");
    Var<Entry> entry(new Entry);
    callback(entry);
    return;
}

} // namespace ooni
} // namespace mk
