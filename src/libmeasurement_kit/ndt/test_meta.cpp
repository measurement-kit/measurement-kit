// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/ndt/test_meta_impl.hpp"

namespace mk {
namespace ndt {
namespace test_meta {

void run(SharedPtr<Context> ctx, Callback<Error> callback) {
    run_impl(ctx, callback);
}

} // namespace test_meta
} // namespace mk
} // namespace ndt
