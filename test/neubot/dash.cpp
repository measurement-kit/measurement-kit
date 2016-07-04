#define CATCH_CONFIG_MAIN

#include "src/ext/Catch/single_include/catch.hpp"
#include <measurement_kit/neubot.hpp>
#include <src/neubot/negotiate_impl.hpp>

using namespace mk::neubot::negotiate;

static void fail(std::string, Callback<Error, mlabns::Reply> cb, Settings,
                 Var<Reactor>, Var<Logger>) {
    cb(MockedError(), {});
}

TEST_CASE("run() deals with mlab-ns query error") {
    run_impl<fail>(
        [](Error err) { REQUIRE(err); }, {},
        Logger::global(), Reactor::global());
}

TEST_CASE("Test works as expected") {
    loop_with_initial_event([=]() {
        run([=](Error error) {
            REQUIRE(!error);
            break_loop();
        });
    });
}
