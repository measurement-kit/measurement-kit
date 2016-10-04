// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../ndt/run_impl.hpp"

namespace mk {
namespace ndt {
namespace scriptable {

using namespace mk::report;

void run(Callback<std::string> callback, Settings settings,
         Var<Runner> runner, Var<Logger> logger) {
    Var<Entry> entry(new Entry);
    ndt::run(entry, [=](Error error) {
        if (error) {
            callback("{}");
            return;
        }
        callback(entry->dump(4));
    }, settings, logger, runner->reactor);
}

} // namespace scriptable
} // namespace mk
} // namespace ndt
