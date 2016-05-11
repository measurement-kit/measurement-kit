// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_NDT_TEST_C2S_HPP
#define SRC_NDT_TEST_C2S_HPP

#include "src/common/utils.hpp"
#include "src/ndt/context.hpp"
#include "src/ndt/definitions.hpp"
#include "src/ndt/messages.hpp"
#include <measurement_kit/ndt.hpp>
#include <measurement_kit/net.hpp>

namespace mk {
namespace ndt {
namespace test_c2s {

using namespace net;

/// Coroutine that does the real c2s test
void coroutine(std::string address, int port, double runtime,
                   Callback<Error, Continuation<Error>> cb, double timeout = 10.0,
                   Var<Logger> logger = Logger::global(),
                   Var<Reactor> reactor = Reactor::global());

/// Run the C2S test
void run(Var<Context> ctx, Callback<Error> callback);

} // namespace test_c2s
} // namespace ndt
} // namespace mk
#endif
