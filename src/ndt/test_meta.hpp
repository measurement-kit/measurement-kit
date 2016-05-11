// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_NDT_TEST_META_HPP
#define SRC_NDT_TEST_META_HPP

#include "src/ext/json/src/json.hpp"
#include "src/ndt/context.hpp"
#include "src/ndt/messages.hpp"
#include <measurement_kit/ndt.hpp>

namespace mk {
namespace ndt {
namespace test_meta {

using namespace mk::net;

/// Run the META test
void run(Var<Context> ctx, Callback<Error> callback);

} // namespace test_meta
} // namespace mk
} // namespace ndt
#endif
