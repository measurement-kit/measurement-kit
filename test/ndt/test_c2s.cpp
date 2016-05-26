// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include "src/ndt/test_c2s_impl.hpp"
#include <measurement_kit/ndt.hpp>

using namespace mk;
using namespace mk::ndt;
using json = nlohmann::json;

static void fail(std::string, int, Callback<Error, Var<Transport>> cb, Settings,
                 Var<Logger>, Var<Reactor>) {
    cb(MockedError(), nullptr);
}

TEST_CASE("coroutine() is robust to connect error") {
    test_c2s::coroutine_impl<fail>(
        "www.google.com", 3301, 10.0,
        [](Error err, Continuation<Error>) { REQUIRE(err == MockedError()); },
        2.0, {}, Logger::global(), Reactor::global());
}

static void fail(Var<Context>, Callback<Error, uint8_t, std::string> cb,
                 Var<Reactor> = Reactor::global()) {
    cb(MockedError(), 0, "");
}

static void invalid(Var<Context>, Callback<Error, uint8_t, std::string> cb,
                    Var<Reactor> = Reactor::global()) {
    cb(NoError(), MSG_ERROR, "");
}

TEST_CASE("run() deals with messages::read() fail") {
    Var<Context> ctx(new Context);
    test_c2s::run_impl<fail>(
        ctx, [](Error err) { REQUIRE(err == ReadingTestPrepareError()); });
}

TEST_CASE("run() deals with receiving message different from PREPARE") {
    Var<Context> ctx(new Context);
    test_c2s::run_impl<invalid>(
        ctx, [](Error err) { REQUIRE(err == NotTestPrepareError()); });
}

static void invalid_port(Var<Context>,
                         Callback<Error, uint8_t, std::string> cb,
                         Var<Reactor> = Reactor::global()) {
    cb(NoError(), TEST_PREPARE, "foobar");
}

TEST_CASE("run() deals with receiving invalid port") {
    Var<Context> ctx(new Context);
    test_c2s::run_impl<invalid_port>(
        ctx, [](Error err) { REQUIRE(err == InvalidPortError()); });
}

static void too_large(Var<Context>, Callback<Error, uint8_t, std::string> cb,
                      Var<Reactor> = Reactor::global()) {
    cb(NoError(), TEST_PREPARE, "65536");
}

TEST_CASE("run() deals with receiving too large port") {
    Var<Context> ctx(new Context);
    test_c2s::run_impl<too_large>(
        ctx, [](Error err) { REQUIRE(err == InvalidPortError()); });
}

static void too_small(Var<Context>, Callback<Error, uint8_t, std::string> cb,
                      Var<Reactor> = Reactor::global()) {
    cb(NoError(), TEST_PREPARE, "-1");
}

TEST_CASE("run() deals with receiving too small port") {
    Var<Context> ctx(new Context);
    test_c2s::run_impl<too_small>(
        ctx, [](Error err) { REQUIRE(err == InvalidPortError()); });
}

static void test_prepare(Var<Context>,
                         Callback<Error, uint8_t, std::string> cb,
                         Var<Reactor> = Reactor::global()) {
    cb(NoError(), TEST_PREPARE, "3010");
}

static void fail(std::string, int, double,
                 Callback<Error, Continuation<Error>> cb, double, Settings,
                 Var<Logger>, Var<Reactor>) {
    cb(MockedError(), [](Callback<Error>) {
        REQUIRE(false); // should not happen
    });
}

TEST_CASE("run() deals with coroutine fail") {
    Var<Context> ctx(new Context);
    test_c2s::run_impl<test_prepare, fail>(
        ctx, [](Error err) { REQUIRE(err == ConnectTestConnectionError()); });
}

static void connect_but_fail_later(std::string, int, double,
                                   Callback<Error, Continuation<Error>> cb,
                                   double, Settings, Var<Logger>,
                                   Var<Reactor>) {
    cb(NoError(), [](Callback<Error> cb) { cb(MockedError()); });
}

TEST_CASE("run() deals with error when reading TEST_START") {
    Var<Context> ctx(new Context);
    test_c2s::run_impl<test_prepare, connect_but_fail_later, fail>(
        ctx, [](Error err) { REQUIRE(err == ReadingTestStartError()); });
}

TEST_CASE("run() deals with unexpected message instead of TEST_START") {
    Var<Context> ctx(new Context);
    test_c2s::run_impl<test_prepare, connect_but_fail_later, invalid>(
        ctx, [](Error err) { REQUIRE(err == NotTestStartError()); });
}

static void test_start(Var<Context>, Callback<Error, uint8_t, std::string> cb,
                       Var<Reactor> = Reactor::global()) {
    cb(NoError(), TEST_START, "");
}

TEST_CASE("run() deals with coroutine terminating with error") {
    Var<Context> ctx(new Context);
    test_c2s::run_impl<test_prepare, connect_but_fail_later, test_start>(
        ctx, [](Error err) { REQUIRE(err == MockedError()); });
}

static void coro_ok(std::string, int, double,
                    Callback<Error, Continuation<Error>> cb, double, Settings,
                    Var<Logger>, Var<Reactor>) {
    cb(NoError(), [](Callback<Error> cb) { cb(NoError()); });
}

TEST_CASE("run() deals with error when reading TEST_MSG") {
    Var<Context> ctx(new Context);
    test_c2s::run_impl<test_prepare, coro_ok, test_start, fail>(
        ctx, [](Error err) { REQUIRE(err == ReadingTestMsgError()); });
}

TEST_CASE("run() deals with unexpected message instead of TEST_MSG") {
    Var<Context> ctx(new Context);
    test_c2s::run_impl<test_prepare, coro_ok, test_start, invalid>(
        ctx, [](Error err) { REQUIRE(err == NotTestMsgError()); });
}

static void test_msg(Var<Context>, Callback<Error, uint8_t, std::string> cb,
                     Var<Reactor> = Reactor::global()) {
    cb(NoError(), TEST_MSG, "");
}

TEST_CASE("run() deals with error when reading TEST_FINALIZE") {
    Var<Context> ctx(new Context);
    test_c2s::run_impl<test_prepare, coro_ok, test_start, test_msg, fail>(
        ctx, [](Error err) { REQUIRE(err == ReadingTestFinalizeError()); });
}

TEST_CASE("run() deals with unexpected message instead of TEST_FINALIZE") {
    Var<Context> ctx(new Context);
    test_c2s::run_impl<test_prepare, coro_ok, test_start, test_msg, invalid>(
        ctx, [](Error err) { REQUIRE(err == NotTestFinalizeError()); });
}
