// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "private/nettests/runnable.hpp"
#include "private/nettests/runner.hpp"

#include <measurement_kit/nettests.hpp>

#include <atomic>
#include <cassert>
#include <future>
#include <thread>

namespace mk {
namespace nettests {

void Runner::start_test(Var<Runnable> test, Callback<Var<Runnable>> fn) {
    // Note: here we MUST force the runnable's reactor to be our reactor
    // otherwise we cannot run the specified test...
    assert(not test->reactor);
    test->reactor = impl_.reactor();
    impl_.start(std::to_string((unsigned long long)test.get()), test->logger,
                [test](Callback<Error> &&cb) {
                    test->begin([=](Error) {
                        // TODO: do not ignore the error
                        test->end([=](Error) {
                            // TODO: do not ignore the error
                            cb(NoError());
                        });
                    });
                },
                [fn, test](const Error && /*error*/) { fn(test); });
}

void Runner::stop() { impl_.stop(); }

bool Runner::empty() { return impl_.active() == 0; }

} // namespace nettests
} // namespace mk
