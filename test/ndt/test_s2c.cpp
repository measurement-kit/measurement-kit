// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "test/winsock.hpp"

#define CATCH_CONFIG_MAIN
#include "src/libmeasurement_kit/ext/catch.hpp"

#include "src/libmeasurement_kit/ndt/test_s2c_impl.hpp"

using namespace mk;
using namespace mk::ndt;

static void failure(std::string, int, int, ConnectManyCb callback,
                    Settings, SharedPtr<Reactor>, SharedPtr<Logger>) {
    callback(MockedError(), {});
}

TEST_CASE("coroutine() is robust to connect error") {
    SharedPtr<Entry> entry{new Entry};
    test_s2c::coroutine_impl<failure>(
        entry, "www.google.com", 3301,
        [](Error err, Continuation<Error, double>) {
            REQUIRE(err == MockedError());
        },
        2.0, {}, Reactor::global(), Logger::global());
}

static void failure(SharedPtr<Context>, Callback<Error, uint8_t, std::string> cb,
                    SharedPtr<Reactor> = Reactor::global()) {
    cb(MockedError(), 0, "");
}

TEST_CASE("finalizing_test() deals with read_msg() error") {
    SharedPtr<Context> ctx(new Context);
    SharedPtr<Entry> entry(new Entry);
    test_s2c::finalizing_test_impl<failure>(
        ctx, entry, [](Error err) { REQUIRE(err == ReadingTestMsgError()); });
}

static void invalid(SharedPtr<Context>, Callback<Error, uint8_t, std::string> cb,
                    SharedPtr<Reactor> = Reactor::global()) {
    cb(NoError(), MSG_ERROR, "");
}

TEST_CASE("finalizing_test() deals with receiving invalid message") {
    SharedPtr<Context> ctx(new Context);
    SharedPtr<Entry> entry(new Entry);
    test_s2c::finalizing_test_impl<invalid>(
        ctx, entry, [](Error err) { REQUIRE(err == NotTestMsgError()); });
}

// XXX: static test function with a state is not good
static void empty(SharedPtr<Context>, Callback<Error, uint8_t, std::string> cb,
                  SharedPtr<Reactor> = Reactor::global()) {
    static int count = 0;
    if (count++ == 0) {
        cb(NoError(), TEST_MSG, "");
    } else {
        cb(NoError(), TEST_FINALIZE, "");
    }
}

TEST_CASE("finalizing_test() deals with receiving invalid json") {
    SharedPtr<Context> ctx(new Context);
    ctx->entry.reset(new Entry);  // Required for when we reach TEST_FINALIZE
    SharedPtr<Entry> entry(new Entry);
    test_s2c::finalizing_test_impl<empty>(
        ctx, entry, [](Error err) { REQUIRE(err == NoError()); });
}

TEST_CASE("run() deals with messages::read() failure") {
    SharedPtr<Context> ctx(new Context);
    test_s2c::run_impl<failure>(
        ctx, [](Error err) { REQUIRE(err == ReadingTestPrepareError()); });
}

TEST_CASE("run() deals with receiving message different from PREPARE") {
    SharedPtr<Context> ctx(new Context);
    test_s2c::run_impl<invalid>(
        ctx, [](Error err) { REQUIRE(err == NotTestPrepareError()); });
}

static void invalid_port(SharedPtr<Context>,
                         Callback<Error, uint8_t, std::string> cb,
                         SharedPtr<Reactor> = Reactor::global()) {
    cb(NoError(), TEST_PREPARE, "foobar");
}

TEST_CASE("run() deals with receiving invalid port") {
    SharedPtr<Context> ctx(new Context);
    test_s2c::run_impl<invalid_port>(
        ctx, [](Error err) { REQUIRE(err == InvalidPortError()); });
}

static void too_large(SharedPtr<Context>, Callback<Error, uint8_t, std::string> cb,
                      SharedPtr<Reactor> = Reactor::global()) {
    cb(NoError(), TEST_PREPARE, "65536");
}

TEST_CASE("run() deals with receiving too large port") {
    SharedPtr<Context> ctx(new Context);
    test_s2c::run_impl<too_large>(
        ctx, [](Error err) { REQUIRE(err == InvalidPortError()); });
}

static void too_small(SharedPtr<Context>, Callback<Error, uint8_t, std::string> cb,
                      SharedPtr<Reactor> = Reactor::global()) {
    cb(NoError(), TEST_PREPARE, "-1");
}

TEST_CASE("run() deals with receiving too small port") {
    SharedPtr<Context> ctx(new Context);
    test_s2c::run_impl<too_small>(
        ctx, [](Error err) { REQUIRE(err == InvalidPortError()); });
}

static void success(SharedPtr<Context>, Callback<Error, uint8_t, std::string> cb,
                    SharedPtr<Reactor> = Reactor::global()) {
    cb(NoError(), TEST_PREPARE, "3010");
}

static void failure(SharedPtr<Entry>, std::string, test_s2c::Params,
                    Callback<Error, Continuation<Error, double>> cb, double,
                    Settings, SharedPtr<Reactor>, SharedPtr<Logger>) {
    cb(MockedError(), [](Callback<Error, double>) {
        REQUIRE(false); // should not happen
    });
}

TEST_CASE("run() deals with coroutine failure") {
    SharedPtr<Context> ctx(new Context);
    test_s2c::run_impl<success, failure>(
        ctx, [](Error err) { REQUIRE(err == ConnectTestConnectionError()); });
}

static void test_prepare(SharedPtr<Context>,
                         Callback<Error, uint8_t, std::string> cb,
                         SharedPtr<Reactor> = Reactor::global()) {
    cb(NoError(), TEST_PREPARE, "3010");
}

static void
connect_but_fail_later(SharedPtr<Entry>, std::string, test_s2c::Params,
                       Callback<Error, Continuation<Error, double>> cb, double,
                       Settings, SharedPtr<Reactor>, SharedPtr<Logger>) {
    cb(NoError(), [](Callback<Error, double> cb) { cb(MockedError(), 0.0); });
}

TEST_CASE("run() deals with error when reading TEST_START") {
    SharedPtr<Context> ctx(new Context);
    test_s2c::run_impl<test_prepare, connect_but_fail_later, failure>(
        ctx, [](Error err) { REQUIRE(err == ReadingTestStartError()); });
}

TEST_CASE("run() deals with unexpected message instead of TEST_START") {
    SharedPtr<Context> ctx(new Context);
    test_s2c::run_impl<test_prepare, connect_but_fail_later, invalid>(
        ctx, [](Error err) { REQUIRE(err == NotTestStartError()); });
}

static void test_start(SharedPtr<Context>, Callback<Error, uint8_t, std::string> cb,
                       SharedPtr<Reactor> = Reactor::global()) {
    cb(NoError(), TEST_START, "");
}

TEST_CASE("run() deals with coroutine terminating with error") {
    SharedPtr<Context> ctx(new Context);
    test_s2c::run_impl<test_prepare, connect_but_fail_later, test_start>(
        ctx, [](Error err) { REQUIRE(err == MockedError()); });
}

static void coro_ok(SharedPtr<Entry>, std::string, test_s2c::Params,
                    Callback<Error, Continuation<Error, double>> cb, double,
                    Settings, SharedPtr<Reactor>, SharedPtr<Logger>) {
    cb(NoError(), [](Callback<Error, double> cb) { cb(NoError(), 0.0); });
}

static void failure(SharedPtr<Context>, Callback<Error, uint8_t, Json> cb,
                    SharedPtr<Reactor> = Reactor::global()) {
    cb(MockedError(), 0, {});
}

TEST_CASE("run() deals with error when reading TEST_MSG") {
    SharedPtr<Context> ctx(new Context);
    test_s2c::run_impl<test_prepare, coro_ok, test_start, failure>(
        ctx, [](Error err) { REQUIRE(err == ReadingTestMsgError()); });
}

static void invalid(SharedPtr<Context>, Callback<Error, uint8_t, Json> cb,
                    SharedPtr<Reactor> = Reactor::global()) {
    cb(NoError(), MSG_ERROR, {});
}

TEST_CASE("run() deals with unexpected message instead of TEST_MSG") {
    SharedPtr<Context> ctx(new Context);
    test_s2c::run_impl<test_prepare, coro_ok, test_start, invalid>(
        ctx, [](Error err) { REQUIRE(err == NotTestMsgError()); });
}

static void msg_test(SharedPtr<Context>, Callback<Error, uint8_t, Json> cb,
                     SharedPtr<Reactor> = Reactor::global()) {
    cb(NoError(), TEST_MSG, {});
}

static ErrorOr<Buffer> failure(std::string) { return {MockedError(), {}}; }

TEST_CASE("run() deals with format_test_msg() failure") {
    SharedPtr<Context> ctx(new Context);
    test_s2c::run_impl<test_prepare, coro_ok, test_start, msg_test, failure>(
        ctx, [](Error err) { REQUIRE(err == SerializingTestMsgError()); });
}

static ErrorOr<Buffer> success(std::string s) {
    Buffer buff;
    buff.write(s);
    return {NoError(), buff};
}

static void failure(SharedPtr<Context>, Buffer, Callback<Error> cb) {
    cb(MockedError());
}

TEST_CASE("run() deals with write() failure") {
    SharedPtr<Context> ctx(new Context);
    test_s2c::run_impl<test_prepare, coro_ok, test_start, msg_test, success,
                       failure>(
        ctx, [](Error err) { REQUIRE(err == WritingTestMsgError()); });
}
