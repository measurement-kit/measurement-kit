// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/ndt/test-c2s.hpp"

namespace mk {
namespace ndt {
namespace tests {

using namespace net;

void c2s_coroutine(std::string address, int port, double runtime,
                   Callback<Error, Continuation<Error>> cb, double timeout,
                   Var<Logger> logger, Var<Reactor> reactor) {
    c2s_coroutine_impl(address, port, runtime, cb, timeout, logger, reactor);
}

void run_test_c2s(Var<Context> ctx, Callback<Error> callback) {
    run_test_c2s_impl(ctx, callback);
}

} // namespace tests
} // namespace ndt
} // namespace mk
