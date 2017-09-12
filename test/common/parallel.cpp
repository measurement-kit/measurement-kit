// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#define CATCH_CONFIG_MAIN

#include "private/ext/catch.hpp"
#include <measurement_kit/common/reactor.hpp>
#include <measurement_kit/common/detail/parallel.hpp>

using namespace mk;

TEST_CASE("mk::parallel() works as expected") {

    SECTION("With no continuations") {
        auto okay = false;
        auto count = 0;
        ParallelExecutor parallel_executor{[&](Error error) {
            okay = (error == NoError());
            count += 1;
        }};
        parallel_executor.start(2);
        REQUIRE(okay);
        REQUIRE(count == 1);
    }

    SECTION("With no parallelism") {
        auto okay = false;
        auto count = 0;
        ParallelExecutor parallel_executor{[&](Error error) {
            okay = (error == ValueError());
            count += 1;
        }};
        parallel_executor.start(0);
        REQUIRE(okay);
        REQUIRE(count == 1);
    }

    auto fn = [](Error expected_err, std::function<Error(size_t)> &&set_err) {
        static constexpr auto delay = 0.1;
        static constexpr auto size = 16;
        Error global_error = MockedError();
        Var<Reactor> reactor = Reactor::make();
        reactor->run_with_initial_event([=, &global_error]() {
            ParallelExecutor parallel_executor{[=, &global_error](Error err) {
                global_error = err;
                reactor->stop();
            }};
            // Implementation note: the following callback does not reference
            // the executor, so we verify that it is possible to create it
            // somewhere on the stack and continue using it also later.
            for (size_t i = 0; i < size; ++i) {
                parallel_executor.add([=](Callback<Error> &&callback) {
                    reactor->call_later(i * delay,
                                        [=]() { callback(set_err(i)); });
                });
            }
            parallel_executor.start(2);
        });
        REQUIRE(global_error == expected_err);
        REQUIRE(global_error.child_errors.size() == size);
        for (size_t i = 0; i < size; ++i) {
            auto sub_error = global_error.child_errors[i];
            REQUIRE(*sub_error == set_err(i));
        }
    };

    SECTION("With all successes") {
        fn(NoError(), [](size_t) { return NoError(); });
    };

    SECTION("With some failures") {
        fn(ParallelOperationError(), [](size_t i) -> Error {
            if ((i % 2) == 0) {
                return MockedError();
            } else {
                return NoError();
            }
        });
    }

    SECTION("With all failures") {
        fn(ParallelOperationError(), [](size_t) { return MockedError(); });
    };
}
