// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <measurement_kit/common.hpp>

using namespace mk;

static void coroutine(Callback<Continuation<>> cb) {
    call_later(1.0, [=]() {
        cb(NoError(), [=](Callback<> cb) {
            call_later(1.0, [=]() {
                cb(NoError());
            });
        });
    });
}

TEST_CASE("The continuation works as expected") {
    loop_with_initial_event([=]() {
        coroutine([=](Error err, Continuation<> cc) {
            REQUIRE(!err);
            cc([=](Error err) {
                REQUIRE(!err);
                break_loop();
            });
        });
    });
}
