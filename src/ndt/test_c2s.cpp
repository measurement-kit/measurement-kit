// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "src/ndt/test_c2s_impl.hpp"

namespace mk {
namespace ndt {
namespace test_c2s {

void coroutine(std::string address, int port, double runtime,
               Callback<Error, Continuation<Error>> cb, double timeout,
               Settings settings, Var<Logger> logger, Var<Reactor> reactor) {
    coroutine_impl(address, port, runtime, cb, timeout, settings, logger,
                   reactor);
}

void run(Var<Context> ctx, Callback<Error> callback) {
    run_impl(ctx, callback);
}

} // namespace test_c2s
} // namespace ndt
} // namespace mk
