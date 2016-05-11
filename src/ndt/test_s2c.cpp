// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/ndt/test_s2c.hpp"

namespace mk {
namespace ndt {
namespace tests {

using namespace net;

void s2c_coroutine(std::string address, int port,
                   Callback<Error, Continuation<Error, double>> cb, double timeout,
                   Var<Logger> logger, Var<Reactor> reactor) {
    s2c_coroutine_impl(address, port, cb, timeout, logger, reactor);
}

void finalizing_test(Var<Context> ctx, Callback<Error> callback) {
    finalizing_test_impl(ctx, callback);
}

void run_test_s2c(Var<Context> ctx, Callback<Error> callback) {
    run_test_s2c_impl(ctx, callback);
}

} // namespace tests
} // namespace ndt
} // namespace mk
