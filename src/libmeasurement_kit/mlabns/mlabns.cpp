// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/mlabns/mlabns_impl.hpp"

namespace mk {
namespace mlabns {

ErrorOr<std::string> as_query(Settings &settings);

void query(std::string tool, Callback<Error, Reply> callback, Settings settings,
           SharedPtr<Reactor> reactor, SharedPtr<Logger> logger) {
    query_impl(tool, callback, settings, reactor, logger);
}

} // namespace mlabns
} // namespace mk
