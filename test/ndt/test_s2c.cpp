// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include "src/ndt/test_s2c_impl.hpp"
#include <measurement_kit/ndt.hpp>

using namespace mk;
using namespace mk::ndt;
using json = nlohmann::json;

static void failure(std::string, int, Callback<Error, Var<Transport>> cb,
                    Settings, Var<Logger>, Var<Reactor>) {
    cb(MockedError(), nullptr);
}

TEST_CASE("coroutine() is robust to connect error") {
    test_s2c::coroutine_impl<failure>(
        "www.google.com", 3301,
        [](Error err, Continuation<Error, double>) {
            REQUIRE(err == MockedError());
        },
        2.0, {}, Logger::global(), Reactor::global());
}

static void failure(Var<Context>, Callback<Error, uint8_t, std::string> cb,
                    Var<Reactor> = Reactor::global()) {
    cb(MockedError(), 0, "");
}

TEST_CASE("finalizing_test() deals with read_msg() error") {
    Var<Context> ctx(new Context);
    test_s2c::finalizing_test_impl<failure>(
        ctx, [](Error err) { REQUIRE(err == ReadingTestMsgError()); });
}

static void invalid(Var<Context>, Callback<Error, uint8_t, std::string> cb,
                    Var<Reactor> = Reactor::global()) {
    cb(NoError(), MSG_ERROR, "");
}

TEST_CASE("finalizing_test() deals with receiving invalid message") {
    Var<Context> ctx(new Context);
    test_s2c::finalizing_test_impl<invalid>(
        ctx, [](Error err) { REQUIRE(err == NotTestMsgError()); });
}

// XXX: static test function with a state is not good
static void empty(Var<Context>, Callback<Error, uint8_t, std::string> cb,
                  Var<Reactor> = Reactor::global()) {
    static int count = 0;
    if (count++ == 0) {
        cb(NoError(), TEST_MSG, "");
    } else {
        cb(NoError(), TEST_FINALIZE, "");
    }
}

TEST_CASE("finalizing_test() deals with receiving invalid json") {
    Var<Context> ctx(new Context);
    test_s2c::finalizing_test_impl<empty>(
        ctx, [](Error err) { REQUIRE(err == NoError()); });
}

TEST_CASE("run() deals with messages::read() failure") {
    Var<Context> ctx(new Context);
    test_s2c::run_impl<failure>(
        ctx, [](Error err) { REQUIRE(err == ReadingTestPrepareError()); });
}

TEST_CASE("run() deals with receiving message different from PREPARE") {
    Var<Context> ctx(new Context);
    test_s2c::run_impl<invalid>(
        ctx, [](Error err) { REQUIRE(err == NotTestPrepareError()); });
}

static void invalid_port(Var<Context>,
                         Callback<Error, uint8_t, std::string> cb,
                         Var<Reactor> = Reactor::global()) {
    cb(NoError(), TEST_PREPARE, "foobar");
}

TEST_CASE("run() deals with receiving invalid port") {
    Var<Context> ctx(new Context);
    test_s2c::run_impl<invalid_port>(
        ctx, [](Error err) { REQUIRE(err == InvalidPortError()); });
}

static void too_large(Var<Context>, Callback<Error, uint8_t, std::string> cb,
                      Var<Reactor> = Reactor::global()) {
    cb(NoError(), TEST_PREPARE, "65536");
}

TEST_CASE("run() deals with receiving too large port") {
    Var<Context> ctx(new Context);
    test_s2c::run_impl<too_large>(
        ctx, [](Error err) { REQUIRE(err == InvalidPortError()); });
}

static void too_small(Var<Context>, Callback<Error, uint8_t, std::string> cb,
                      Var<Reactor> = Reactor::global()) {
    cb(NoError(), TEST_PREPARE, "-1");
}

TEST_CASE("run() deals with receiving too small port") {
    Var<Context> ctx(new Context);
    test_s2c::run_impl<too_small>(
        ctx, [](Error err) { REQUIRE(err == InvalidPortError()); });
}

static void success(Var<Context>, Callback<Error, uint8_t, std::string> cb,
                    Var<Reactor> = Reactor::global()) {
    cb(NoError(), TEST_PREPARE, "3010");
}

static void failure(std::string, int,
                    Callback<Error, Continuation<Error, double>> cb, double,
                    Settings, Var<Logger>, Var<Reactor>) {
    cb(MockedError(), [](Callback<Error, double>) {
        REQUIRE(false); // should not happen
    });
}

TEST_CASE("run() deals with coroutine failure") {
    Var<Context> ctx(new Context);
    test_s2c::run_impl<success, failure>(
        ctx, [](Error err) { REQUIRE(err == ConnectTestConnectionError()); });
}

static void test_prepare(Var<Context>,
                         Callback<Error, uint8_t, std::string> cb,
                         Var<Reactor> = Reactor::global()) {
    cb(NoError(), TEST_PREPARE, "3010");
}

static void
connect_but_fail_later(std::string, int,
                       Callback<Error, Continuation<Error, double>> cb, double,
                       Settings, Var<Logger>, Var<Reactor>) {
    cb(NoError(), [](Callback<Error, double> cb) { cb(MockedError(), 0.0); });
}

TEST_CASE("run() deals with error when reading TEST_START") {
    Var<Context> ctx(new Context);
    test_s2c::run_impl<test_prepare, connect_but_fail_later, failure>(
        ctx, [](Error err) { REQUIRE(err == ReadingTestStartError()); });
}

TEST_CASE("run() deals with unexpected message instead of TEST_START") {
    Var<Context> ctx(new Context);
    test_s2c::run_impl<test_prepare, connect_but_fail_later, invalid>(
        ctx, [](Error err) { REQUIRE(err == NotTestStartError()); });
}

static void test_start(Var<Context>, Callback<Error, uint8_t, std::string> cb,
                       Var<Reactor> = Reactor::global()) {
    cb(NoError(), TEST_START, "");
}

TEST_CASE("run() deals with coroutine terminating with error") {
    Var<Context> ctx(new Context);
    test_s2c::run_impl<test_prepare, connect_but_fail_later, test_start>(
        ctx, [](Error err) { REQUIRE(err == MockedError()); });
}

static void coro_ok(std::string, int,
                    Callback<Error, Continuation<Error, double>> cb, double,
                    Settings, Var<Logger>, Var<Reactor>) {
    cb(NoError(), [](Callback<Error, double> cb) { cb(NoError(), 0.0); });
}

static void failure(Var<Context>, Callback<Error, uint8_t, json> cb,
                    Var<Reactor> = Reactor::global()) {
    cb(MockedError(), 0, {});
}

TEST_CASE("run() deals with error when reading TEST_MSG") {
    Var<Context> ctx(new Context);
    test_s2c::run_impl<test_prepare, coro_ok, test_start, failure>(
        ctx, [](Error err) { REQUIRE(err == ReadingTestMsgError()); });
}

static void invalid(Var<Context>, Callback<Error, uint8_t, json> cb,
                    Var<Reactor> = Reactor::global()) {
    cb(NoError(), MSG_ERROR, {});
}

TEST_CASE("run() deals with unexpected message instead of TEST_MSG") {
    Var<Context> ctx(new Context);
    test_s2c::run_impl<test_prepare, coro_ok, test_start, invalid>(
        ctx, [](Error err) { REQUIRE(err == NotTestMsgError()); });
}

static void msg_test(Var<Context>, Callback<Error, uint8_t, json> cb,
                     Var<Reactor> = Reactor::global()) {
    cb(NoError(), TEST_MSG, {});
}

static ErrorOr<Buffer> failure(std::string) { return MockedError(); }

TEST_CASE("run() deals with format_test_msg() failure") {
    Var<Context> ctx(new Context);
    test_s2c::run_impl<test_prepare, coro_ok, test_start, msg_test, failure>(
        ctx, [](Error err) { REQUIRE(err == SerializingTestMsgError()); });
}

static ErrorOr<Buffer> success(std::string s) {
    Buffer buff;
    buff.write(s);
    return buff;
}

static void failure(Var<Context>, Buffer, Callback<Error> cb) {
    cb(MockedError());
}

TEST_CASE("run() deals with write() failure") {
    Var<Context> ctx(new Context);
    test_s2c::run_impl<test_prepare, coro_ok, test_start, msg_test, success,
                       failure>(
        ctx, [](Error err) { REQUIRE(err == WritingTestMsgError()); });
}
