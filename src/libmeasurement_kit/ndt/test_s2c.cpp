// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "private/ndt/test_s2c_impl.hpp"

namespace mk {
namespace ndt {
namespace test_s2c {

using namespace mk::report;

void coroutine(Var<Entry> report_entry, std::string address, Params params,
               Callback<Error, Continuation<Error, double>> cb, double timeout,
               Settings settings, Reactor reactor,
               Var<Logger> logger) {
    coroutine_impl(report_entry, address, params, cb, timeout, settings,
                   reactor, logger);
}

void finalizing_test(Var<Context> ctx, Var<Entry> cur_entry,
                     Callback<Error> callback) {
    finalizing_test_impl(ctx, cur_entry, callback);
}

void run(Var<Context> ctx, Callback<Error> callback) {
    run_impl(ctx, callback);
}

} // namespace test_s2c
} // namespace ndt
} // namespace mk
