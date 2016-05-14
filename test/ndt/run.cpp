// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include "src/ndt/run_impl.hpp"
#include <measurement_kit/ndt.hpp>

using namespace mk;
using namespace mk::ndt;

static void success(Var<Context>, Callback<Error> cb) { cb(NoError()); }
static void failure(Var<Context>, Callback<Error> cb) { cb(GenericError()); }
static void die(Var<Context>, Callback<Error>) { REQUIRE(false); }

TEST_CASE("We deal with connect error") {
    run_with_specific_server_impl<failure, die, die, die, die, die, die, die,
                                  die, protocol::disconnect_and_callback>(
        "127.0.0.1", 3001, [](Error err) { REQUIRE(err == GenericError()); },
        nullptr, nullptr);
}

TEST_CASE("We deal with send-login error") {
    run_with_specific_server_impl<success, failure, die, die, die, die, die,
                                  die, die, protocol::disconnect_and_callback>(
        "127.0.0.1", 3001, [](Error err) { REQUIRE(err == GenericError()); },
        nullptr, nullptr);
}

TEST_CASE("We deal with recv-and-ignore-kickoff error") {
    run_with_specific_server_impl<success, success, failure, die, die, die, die,
                                  die, die, protocol::disconnect_and_callback>(
        "127.0.0.1", 3001, [](Error err) { REQUIRE(err == GenericError()); },
        nullptr, nullptr);
}

TEST_CASE("We deal with wait-in-queue error") {
    run_with_specific_server_impl<success, success, success, failure, die, die,
                                  die, die, die,
                                  protocol::disconnect_and_callback>(
        "127.0.0.1", 3001, [](Error err) { REQUIRE(err == GenericError()); },
        nullptr, nullptr);
}

TEST_CASE("We deal with recv-version error") {
    run_with_specific_server_impl<success, success, success, success, failure,
                                  die, die, die, die,
                                  protocol::disconnect_and_callback>(
        "127.0.0.1", 3001, [](Error err) { REQUIRE(err == GenericError()); },
        nullptr, nullptr);
}

TEST_CASE("We deal with recv-tests-id error") {
    run_with_specific_server_impl<success, success, success, success, success,
                                  failure, die, die, die,
                                  protocol::disconnect_and_callback>(
        "127.0.0.1", 3001, [](Error err) { REQUIRE(err == GenericError()); },
        nullptr, nullptr);
}

TEST_CASE("We deal with run-tests error") {
    run_with_specific_server_impl<success, success, success, success, success,
                                  success, failure, die, die,
                                  protocol::disconnect_and_callback>(
        "127.0.0.1", 3001, [](Error err) { REQUIRE(err == GenericError()); },
        nullptr, nullptr);
}

TEST_CASE("We deal with recv-results-and-logout error") {
    run_with_specific_server_impl<success, success, success, success, success,
                                  success, success, failure, die,
                                  protocol::disconnect_and_callback>(
        "127.0.0.1", 3001, [](Error err) { REQUIRE(err == GenericError()); },
        nullptr, nullptr);
}
