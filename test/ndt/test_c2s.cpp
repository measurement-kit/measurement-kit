// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "test/winsock.hpp"

#include "include/private/catch.hpp"

#include "src/libmeasurement_kit/ndt/test_c2s_impl.hpp"

using namespace mk;
using namespace mk::ndt;

static void fail(std::string, int, Callback<Error, SharedPtr<Transport>> cb, Settings,
                 SharedPtr<Reactor>, SharedPtr<Logger>) {
    cb(MockedError(), nullptr);
}

TEST_CASE("coroutine() is robust to connect error") {
    SharedPtr<Entry> entry{new Entry};
    test_c2s::coroutine_impl<fail>(
        entry, "www.google.com", 3301, 10.0,
        [](Error err, Continuation<Error>) { REQUIRE(err == MockedError()); },
        2.0, {}, Reactor::global(), Logger::global());
}

static void fail(SharedPtr<Context>, Callback<Error, uint8_t, std::string> cb,
                 SharedPtr<Reactor> = Reactor::global()) {
    cb(MockedError(), 0, "");
}

static void invalid(SharedPtr<Context>, Callback<Error, uint8_t, std::string> cb,
                    SharedPtr<Reactor> = Reactor::global()) {
    cb(NoError(), MSG_ERROR, "");
}

TEST_CASE("run() deals with messages::read() fail") {
    SharedPtr<Context> ctx(new Context);
    test_c2s::run_impl<fail>(
        ctx, [](Error err) { REQUIRE(err == ReadingTestPrepareError()); });
}

TEST_CASE("run() deals with receiving message different from PREPARE") {
    SharedPtr<Context> ctx(new Context);
    test_c2s::run_impl<invalid>(
        ctx, [](Error err) { REQUIRE(err == NotTestPrepareError()); });
}

static void invalid_port(SharedPtr<Context>,
                         Callback<Error, uint8_t, std::string> cb,
                         SharedPtr<Reactor> = Reactor::global()) {
    cb(NoError(), TEST_PREPARE, "foobar");
}

TEST_CASE("run() deals with receiving invalid port") {
    SharedPtr<Context> ctx(new Context);
    test_c2s::run_impl<invalid_port>(
        ctx, [](Error err) { REQUIRE(err == InvalidPortError()); });
}

static void too_large(SharedPtr<Context>, Callback<Error, uint8_t, std::string> cb,
                      SharedPtr<Reactor> = Reactor::global()) {
    cb(NoError(), TEST_PREPARE, "65536");
}

TEST_CASE("run() deals with receiving too large port") {
    SharedPtr<Context> ctx(new Context);
    test_c2s::run_impl<too_large>(
        ctx, [](Error err) { REQUIRE(err == InvalidPortError()); });
}

static void too_small(SharedPtr<Context>, Callback<Error, uint8_t, std::string> cb,
                      SharedPtr<Reactor> = Reactor::global()) {
    cb(NoError(), TEST_PREPARE, "-1");
}

TEST_CASE("run() deals with receiving too small port") {
    SharedPtr<Context> ctx(new Context);
    test_c2s::run_impl<too_small>(
        ctx, [](Error err) { REQUIRE(err == InvalidPortError()); });
}

static void test_prepare(SharedPtr<Context>,
                         Callback<Error, uint8_t, std::string> cb,
                         SharedPtr<Reactor> = Reactor::global()) {
    cb(NoError(), TEST_PREPARE, "3010");
}

static void fail(SharedPtr<Entry>, std::string, int, double,
                 Callback<Error, Continuation<Error>> cb, double, Settings,
                 SharedPtr<Reactor>, SharedPtr<Logger>) {
    cb(MockedError(), [](Callback<Error>) {
        REQUIRE(false); // should not happen
    });
}

TEST_CASE("run() deals with coroutine fail") {
    SharedPtr<Context> ctx(new Context);
    test_c2s::run_impl<test_prepare, fail>(
        ctx, [](Error err) { REQUIRE(err == ConnectTestConnectionError()); });
}

static void connect_but_fail_later(SharedPtr<Entry>, std::string, int, double,
                                   Callback<Error, Continuation<Error>> cb,
                                   double, Settings, SharedPtr<Reactor>,
                                   SharedPtr<Logger>) {
    cb(NoError(), [](Callback<Error> cb) { cb(MockedError()); });
}

TEST_CASE("run() deals with error when reading TEST_START") {
    SharedPtr<Context> ctx(new Context);
    test_c2s::run_impl<test_prepare, connect_but_fail_later, fail>(
        ctx, [](Error err) { REQUIRE(err == ReadingTestStartError()); });
}

TEST_CASE("run() deals with unexpected message instead of TEST_START") {
    SharedPtr<Context> ctx(new Context);
    test_c2s::run_impl<test_prepare, connect_but_fail_later, invalid>(
        ctx, [](Error err) { REQUIRE(err == NotTestStartError()); });
}

static void test_start(SharedPtr<Context>, Callback<Error, uint8_t, std::string> cb,
                       SharedPtr<Reactor> = Reactor::global()) {
    cb(NoError(), TEST_START, "");
}

TEST_CASE("run() deals with coroutine terminating with error") {
    SharedPtr<Context> ctx(new Context);
    test_c2s::run_impl<test_prepare, connect_but_fail_later, test_start>(
        ctx, [](Error err) { REQUIRE(err == MockedError()); });
}

static void coro_ok(SharedPtr<Entry>, std::string, int, double,
                    Callback<Error, Continuation<Error>> cb, double, Settings,
                    SharedPtr<Reactor>, SharedPtr<Logger>) {
    cb(NoError(), [](Callback<Error> cb) { cb(NoError()); });
}

TEST_CASE("run() deals with error when reading TEST_MSG") {
    SharedPtr<Context> ctx(new Context);
    test_c2s::run_impl<test_prepare, coro_ok, test_start, fail>(
        ctx, [](Error err) { REQUIRE(err == ReadingTestMsgError()); });
}

TEST_CASE("run() deals with unexpected message instead of TEST_MSG") {
    SharedPtr<Context> ctx(new Context);
    test_c2s::run_impl<test_prepare, coro_ok, test_start, invalid>(
        ctx, [](Error err) { REQUIRE(err == NotTestMsgError()); });
}

static void test_msg(SharedPtr<Context>, Callback<Error, uint8_t, std::string> cb,
                     SharedPtr<Reactor> = Reactor::global()) {
    cb(NoError(), TEST_MSG, "");
}

TEST_CASE("run() deals with error when reading TEST_FINALIZE") {
    SharedPtr<Context> ctx(new Context);
    ctx->entry.reset(new Entry);
    test_c2s::run_impl<test_prepare, coro_ok, test_start, test_msg, fail>(
        ctx, [](Error err) { REQUIRE(err == ReadingTestFinalizeError()); });
}

TEST_CASE("run() deals with unexpected message instead of TEST_FINALIZE") {
    SharedPtr<Context> ctx(new Context);
    ctx->entry.reset(new Entry);
    test_c2s::run_impl<test_prepare, coro_ok, test_start, test_msg, invalid>(
        ctx, [](Error err) { REQUIRE(err == NotTestFinalizeError()); });
}
