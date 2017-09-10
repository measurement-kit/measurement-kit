// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "private/ndt/test_c2s_impl.hpp"

namespace mk {
namespace ndt {
namespace test_c2s {

void coroutine(Var<Entry> e, std::string address, int port, double runtime,
               Callback<Error, Continuation<Error>> cb, double timeout,
               Settings settings, Reactor reactor,
               Var<Logger> logger) {
    coroutine_impl(e, address, port, runtime, cb, timeout, settings, reactor,
                   logger);
}

void run(Var<Context> ctx, Callback<Error> callback) {
    run_impl(ctx, callback);
}

} // namespace test_c2s
} // namespace ndt
} // namespace mk
