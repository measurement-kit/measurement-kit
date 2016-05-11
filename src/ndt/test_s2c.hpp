// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_NDT_TEST_S2C_HPP
#define SRC_NDT_TEST_S2C_HPP

#include "src/common/utils.hpp"
#include "src/ndt/context.hpp"
#include "src/ndt/definitions.hpp"
#include "src/ndt/messages.hpp"
#include <measurement_kit/ndt.hpp>
#include <measurement_kit/net.hpp>

namespace mk {
namespace ndt {
namespace test_s2c {

using json = nlohmann::json;
using namespace net;

/// Coroutine that does the real s2c test
void coroutine(std::string address, int port,
                   Callback<Error, Continuation<Error, double>> cb, double timeout = 10.0,
                   Var<Logger> logger = Logger::global(),
                   Var<Reactor> reactor = Reactor::global());

/// Final state of this test
void finalizing_test(Var<Context> ctx, Callback<Error> callback);

/// Run the S2C test
void run(Var<Context> ctx, Callback<Error> callback);

} // namespace test_s2c
} // namespace ndt
} // namespace mk
#endif
