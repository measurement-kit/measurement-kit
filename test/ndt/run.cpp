// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include "src/ndt/run_impl.hpp"
#include <measurement_kit/ndt.hpp>

using namespace mk;
using namespace mk::ndt;

/*
             _ _
 _   _ _ __ (_) |_
| | | | '_ \| | __|
| |_| | | | | | |_
 \__,_|_| |_|_|\__|

*/

static void success(Var<Context>, Callback<Error> cb) { cb(NoError()); }
static void failure(Var<Context>, Callback<Error> cb) { cb(MockedError()); }
static void die(Var<Context>, Callback<Error>) { REQUIRE(false); }

TEST_CASE("We deal with connect error") {
    run_with_specific_server_impl<failure, die, die, die, die, die, die, die,
                                  die, protocol::disconnect_and_callback>(
        "127.0.0.1", 3001, [](Error err) { REQUIRE(err == MockedError()); }, {},
        Logger::global(), Reactor::global());
}

TEST_CASE("We deal with send-login error") {
    run_with_specific_server_impl<success, failure, die, die, die, die, die,
                                  die, die, protocol::disconnect_and_callback>(
        "127.0.0.1", 3001, [](Error err) { REQUIRE(err == MockedError()); }, {},
        Logger::global(), Reactor::global());
}

TEST_CASE("We deal with recv-and-ignore-kickoff error") {
    run_with_specific_server_impl<success, success, failure, die, die, die, die,
                                  die, die, protocol::disconnect_and_callback>(
        "127.0.0.1", 3001, [](Error err) { REQUIRE(err == MockedError()); }, {},
        Logger::global(), Reactor::global());
}

TEST_CASE("We deal with wait-in-queue error") {
    run_with_specific_server_impl<success, success, success, failure, die, die,
                                  die, die, die,
                                  protocol::disconnect_and_callback>(
        "127.0.0.1", 3001, [](Error err) { REQUIRE(err == MockedError()); }, {},
        Logger::global(), Reactor::global());
}

TEST_CASE("We deal with recv-version error") {
    run_with_specific_server_impl<success, success, success, success, failure,
                                  die, die, die, die,
                                  protocol::disconnect_and_callback>(
        "127.0.0.1", 3001, [](Error err) { REQUIRE(err == MockedError()); }, {},
        Logger::global(), Reactor::global());
}

TEST_CASE("We deal with recv-tests-id error") {
    run_with_specific_server_impl<success, success, success, success, success,
                                  failure, die, die, die,
                                  protocol::disconnect_and_callback>(
        "127.0.0.1", 3001, [](Error err) { REQUIRE(err == MockedError()); }, {},
        Logger::global(), Reactor::global());
}

TEST_CASE("We deal with run-tests error") {
    run_with_specific_server_impl<success, success, success, success, success,
                                  success, failure, die, die,
                                  protocol::disconnect_and_callback>(
        "127.0.0.1", 3001, [](Error err) { REQUIRE(err == MockedError()); }, {},
        Logger::global(), Reactor::global());
}

TEST_CASE("We deal with recv-results-and-logout error") {
    run_with_specific_server_impl<success, success, success, success, success,
                                  success, success, failure, die,
                                  protocol::disconnect_and_callback>(
        "127.0.0.1", 3001, [](Error err) { REQUIRE(err == MockedError()); }, {},
        Logger::global(), Reactor::global());
}

TEST_CASE("run() deals with invalid port error") {
    run([](Error err) { REQUIRE(err == InvalidPortError()); },
        {
            {"port", "xo"},
        });
}

static void fail(std::string, Callback<Error, mlabns::Reply> cb, Settings,
                 Var<Reactor>, Var<Logger>) {
    cb(MockedError(), {});
}

TEST_CASE("run() deals with mlab-ns query error") {
    run_impl<run_with_specific_server, fail>(
        [](Error err) { REQUIRE(err == MlabnsQueryError()); }, {},
        Logger::global(), Reactor::global());
}

/*
 _       _                       _   _
(_)_ __ | |_ ___  __ _ _ __ __ _| |_(_) ___  _ __
| | '_ \| __/ _ \/ _` | '__/ _` | __| |/ _ \| '_ \
| | | | | ||  __/ (_| | | | (_| | |_| | (_) | | | |
|_|_| |_|\__\___|\__, |_|  \__,_|\__|_|\___/|_| |_|
                 |___/
*/

TEST_CASE("NDT test run() should work") {
    loop_with_initial_event([]() {
        set_verbosity(MK_LOG_INFO);
        ndt::run([](Error err) {
            REQUIRE(!err);
            break_loop();
        });
    });
}
