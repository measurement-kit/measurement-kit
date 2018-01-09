// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/libmeasurement_kit/ext/catch.hpp"

#include <measurement_kit/common.hpp>

using namespace mk;

static void coroutine(SharedPtr<Reactor> reactor, Callback<Error, Continuation<Error>> cb) {
    // Pretend to do some work
    reactor->call_later(1.0, [=]() {

        // Transfer control back to parent and wait for it to restart us
        cb(NoError(), [=](Callback<Error> cb) {

            // Pretend again to do some work
            reactor->call_later(1.0, [=]() {

                // Transfer one second and final time control to parent
                cb(NoError());
            });
        });
    });
}

TEST_CASE("The continuation works as expected") {
    SharedPtr<Reactor> reactor = Reactor::make();
    reactor->run_with_initial_event([=]() {

        // Spawn the coroutine and wait for it to pause
        coroutine(reactor, [=](Error err, Continuation<Error> cc) {
            REQUIRE(!err);

            // Resume the coroutine and wait for it to complete
            cc([=](Error err) {
                REQUIRE(!err);

                // Coroutine complete get out of here
                reactor->stop();
            });
        });
    });
}
