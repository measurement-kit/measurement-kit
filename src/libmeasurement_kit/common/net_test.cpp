// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <chrono>
#include <functional>
#include <measurement_kit/common.hpp>
#include <ratio>
#include <thread>
#include <future>

namespace mk {

void NetTest::run() {
    std::promise<bool> promise;
    std::future<bool> future = promise.get_future();
    run([&promise]() { promise.set_value(true);});
    future.wait();
}

void NetTest::run(Var<Reactor> r) {
    // Because the `reactor` attribute is public, I don't like clobbering
    // its value for no good reason, so do the save and restore dance
    Var<Reactor> old = reactor;
    reactor = r;
    logger->debug("starting up the event loop");
    // XXX the following code throws an error if the reactor is already
    // running because the event loop cannot be reentered. Currently there
    // is no way to cleanly notice of this error an gently repair.
    reactor->loop_with_initial_event([=]() {
        logger->debug("starting up the network test");
        begin([=](Error) {
            logger->debug("finishing the network test");
            end([=](Error) {
                logger->debug("breaking out of the event loop");
                reactor->break_loop();
            });
        });
    });
    logger->debug("out of the event loop");
    reactor = old;
}

void NetTest::run(std::function<void()> callback) {
    Runner::global()->run_test(create_test_(),
                              [=](Var<NetTest>) { callback(); });
}

NetTest::~NetTest() {}

} // namespace mk
