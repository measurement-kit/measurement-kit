// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/mlabns/mlabns_impl.hpp"

namespace mk {
namespace mlabns {

ErrorOr<std::string> as_query(Settings &settings);

void query(std::string tool, Callback<Error, Reply> callback, Settings settings,
           Var<Reactor> reactor, Var<Logger> logger) {
    query_impl(tool, callback, settings, reactor, logger);
}

} // namespace mlabns
} // namespace mk
