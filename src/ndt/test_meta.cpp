// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/ndt/test_meta.hpp"

namespace mk {
namespace ndt {
namespace tests {

void run_test_meta(Var<Context> ctx, Callback<Error> callback) {
    run_test_meta_impl(ctx, callback);
}

} // namespace tests
} // namespace mk
} // namespace ndt
