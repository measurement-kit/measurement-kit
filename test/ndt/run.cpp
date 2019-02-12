// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "test/winsock.hpp"

#include "include/private/catch.hpp"

#include "src/libmeasurement_kit/ndt/run_impl.hpp"

using namespace mk;
using namespace mk::ndt;

/*
             _ _
 _   _ _ __ (_) |_
| | | | '_ \| | __|
| |_| | | | | | |_
 \__,_|_| |_|_|\__|

*/

static void success(SharedPtr<Context>, Callback<Error> cb) { cb(NoError()); }
static void failure(SharedPtr<Context>, Callback<Error> cb) { cb(MockedError()); }
static void die(SharedPtr<Context>, Callback<Error>) { REQUIRE(false); }

TEST_CASE("We deal with connect error") {
    SharedPtr<nlohmann::json> entry{new nlohmann::json};
    run_with_specific_server_impl<failure, die, die, die, die, die, die, die,
                                  die, protocol::disconnect_and_callback>(
        entry, "127.0.0.1", 3001, [](Error err) { REQUIRE(err == MockedError()); }, {},
        Reactor::global(), Logger::global());
}

TEST_CASE("We deal with send-login error") {
    SharedPtr<nlohmann::json> entry{new nlohmann::json};
    run_with_specific_server_impl<success, failure, die, die, die, die, die,
                                  die, die, protocol::disconnect_and_callback>(
        entry, "127.0.0.1", 3001, [](Error err) { REQUIRE(err == MockedError()); }, {},
        Reactor::global(), Logger::global());
}

TEST_CASE("We deal with recv-and-ignore-kickoff error") {
    SharedPtr<nlohmann::json> entry{new nlohmann::json};
    run_with_specific_server_impl<success, success, failure, die, die, die, die,
                                  die, die, protocol::disconnect_and_callback>(
        entry, "127.0.0.1", 3001, [](Error err) { REQUIRE(err == MockedError()); }, {},
        Reactor::global(), Logger::global());
}

TEST_CASE("We deal with wait-in-queue error") {
    SharedPtr<nlohmann::json> entry{new nlohmann::json};
    run_with_specific_server_impl<success, success, success, failure, die, die,
                                  die, die, die,
                                  protocol::disconnect_and_callback>(
        entry, "127.0.0.1", 3001, [](Error err) { REQUIRE(err == MockedError()); }, {},
        Reactor::global(), Logger::global());
}

TEST_CASE("We deal with recv-version error") {
    SharedPtr<nlohmann::json> entry{new nlohmann::json};
    run_with_specific_server_impl<success, success, success, success, failure,
                                  die, die, die, die,
                                  protocol::disconnect_and_callback>(
        entry, "127.0.0.1", 3001, [](Error err) { REQUIRE(err == MockedError()); }, {},
        Reactor::global(), Logger::global());
}

TEST_CASE("We deal with recv-tests-id error") {
    SharedPtr<nlohmann::json> entry{new nlohmann::json};
    run_with_specific_server_impl<success, success, success, success, success,
                                  failure, die, die, die,
                                  protocol::disconnect_and_callback>(
        entry, "127.0.0.1", 3001, [](Error err) { REQUIRE(err == MockedError()); }, {},
        Reactor::global(), Logger::global());
}

TEST_CASE("We deal with run-tests error") {
    SharedPtr<nlohmann::json> entry{new nlohmann::json};
    run_with_specific_server_impl<success, success, success, success, success,
                                  success, failure, die, die,
                                  protocol::disconnect_and_callback>(
        entry, "127.0.0.1", 3001, [](Error err) { REQUIRE(err == MockedError()); }, {},
        Reactor::global(), Logger::global());
}

TEST_CASE("We deal with recv-results-and-logout error") {
    SharedPtr<nlohmann::json> entry{new nlohmann::json};
    run_with_specific_server_impl<success, success, success, success, success,
                                  success, success, failure, die,
                                  protocol::disconnect_and_callback>(
        entry, "127.0.0.1", 3001, [](Error err) { REQUIRE(err == MockedError()); }, {},
        Reactor::make(), Logger::make());
}

TEST_CASE("run() deals with invalid port error") {
    SharedPtr<nlohmann::json> entry{new nlohmann::json};
    run(entry, [](Error err) { REQUIRE(err == InvalidPortError()); },
        {
            {"port", "xo"},
        }, Reactor::make(), Logger::make());
}

static void fail(std::string, Callback<Error, mlabns::Reply> cb, Settings,
                 SharedPtr<Reactor>, SharedPtr<Logger>) {
    cb(MockedError(), mlabns::Reply());
}

TEST_CASE("run() deals with mlab-ns query error") {
    SharedPtr<nlohmann::json> entry{new nlohmann::json};
    run_impl<run_with_specific_server, fail>(
        entry, [](Error err) { REQUIRE(err == MlabnsQueryError()); }, {},
        Reactor::make(), Logger::make());
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
    SharedPtr<nlohmann::json> entry{new nlohmann::json};
    SharedPtr<Reactor> reactor = Reactor::make();
    Settings settings;
    settings["net/ca_bundle_path"] = "cacert.pem";
    reactor->run_with_initial_event([=]() {
        ndt::run(entry, [=](Error err) {
            REQUIRE(!err);
            reactor->stop();
        }, settings, reactor, Logger::make());
    });
}
