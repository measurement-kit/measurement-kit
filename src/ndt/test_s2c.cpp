// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/ndt/test_s2c_impl.hpp"

namespace mk {
namespace ndt {
namespace test_s2c {

void coroutine(std::string address, int port,
               Callback<Error, Continuation<Error, double>> cb, double timeout,
               Settings settings, Var<Logger> logger, Var<Reactor> reactor) {
    coroutine_impl(address, port, cb, timeout, settings, logger, reactor);
}

void finalizing_test(Var<Context> ctx, Callback<Error> callback) {
    finalizing_test_impl(ctx, callback);
}

void run(Var<Context> ctx, Callback<Error> callback) {
    run_impl(ctx, callback);
}

} // namespace test_s2c
} // namespace ndt
} // namespace mk
