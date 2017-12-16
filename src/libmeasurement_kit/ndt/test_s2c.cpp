// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/ndt/test_s2c_impl.hpp"

namespace mk {
namespace ndt {
namespace test_s2c {

using namespace mk::report;

void coroutine(SharedPtr<Entry> report_entry, std::string address, Params params,
               Callback<Error, Continuation<Error, double>> cb, double timeout,
               Settings settings, SharedPtr<Reactor> reactor,
               SharedPtr<Logger> logger) {
    coroutine_impl(report_entry, address, params, cb, timeout, settings,
                   reactor, logger);
}

void finalizing_test(SharedPtr<Context> ctx, SharedPtr<Entry> cur_entry,
                     Callback<Error> callback) {
    finalizing_test_impl(ctx, cur_entry, callback);
}

void run(SharedPtr<Context> ctx, Callback<Error> callback) {
    run_impl(ctx, callback);
}

} // namespace test_s2c
} // namespace ndt
} // namespace mk
