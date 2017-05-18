// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../common/utils.hpp"
#include "../ooni/constants.hpp"
#include "../ooni/utils.hpp"

namespace mk {
namespace ooni {

using namespace mk::report;

void captive_portal(std::string input, Settings options,
                    Callback<Var<Entry>> callback, Var<Reactor> reactor,
                    Var<Logger> logger) {
    Var<Entry> entry(new Entry);
    callback(entry);
}

} // namespace ooni
} // namespace mk
