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
    cb(GenericError(), nullptr);
}

TEST_CASE("coroutine() is robust to connect error") {
    test_s2c::coroutine_impl<failure>(
        "www.google.com", 3301,
        [](Error err, Continuation<Error, double>) {
            REQUIRE(err == GenericError());
        },
        2.0, {}, Logger::global(), Reactor::global());
}

static void failure(Var<Context>, Callback<Error, uint8_t, std::string> cb) {
    cb(GenericError(), 0, "");
}

TEST_CASE("finalizing_test() deals with read_ndt() error") {
    Var<Context> ctx(new Context);
    test_s2c::finalizing_test_impl<failure>(
        ctx, [](Error err) { REQUIRE(err == GenericError()); });
}

static void invalid(Var<Context>, Callback<Error, uint8_t, std::string> cb) {
    cb(NoError(), MSG_ERROR, "");
}

TEST_CASE("finalizing_test() deals with receiving invalid message") {
    Var<Context> ctx(new Context);
    test_s2c::finalizing_test_impl<invalid>(
        ctx, [](Error err) { REQUIRE(err == GenericError()); });
}

static int empty_counter = 0;
static void empty(Var<Context>, Callback<Error, uint8_t, std::string> cb) {
    if (empty_counter++ == 0) {
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
        ctx, [](Error err) { REQUIRE(err == GenericError()); });
}

TEST_CASE("run() deals with receiving message different from PREPARE") {
    Var<Context> ctx(new Context);
    test_s2c::run_impl<invalid>(
        ctx, [](Error err) { REQUIRE(err == GenericError()); });
}

static void invalid_port(Var<Context>,
                         Callback<Error, uint8_t, std::string> cb) {
    cb(NoError(), TEST_PREPARE, "foobar");
}

TEST_CASE("run() deals with receiving invalid port") {
    Var<Context> ctx(new Context);
    test_s2c::run_impl<invalid_port>(
        ctx, [](Error err) { REQUIRE(err == GenericError()); });
}

static void too_large(Var<Context>, Callback<Error, uint8_t, std::string> cb) {
    cb(NoError(), TEST_PREPARE, "65536");
}

TEST_CASE("run() deals with receiving too large port") {
    Var<Context> ctx(new Context);
    test_s2c::run_impl<too_large>(
        ctx, [](Error err) { REQUIRE(err == GenericError()); });
}

static void too_small(Var<Context>, Callback<Error, uint8_t, std::string> cb) {
    cb(NoError(), TEST_PREPARE, "-1");
}

TEST_CASE("run() deals with receiving too small port") {
    Var<Context> ctx(new Context);
    test_s2c::run_impl<too_small>(
        ctx, [](Error err) { REQUIRE(err == GenericError()); });
}

static void success(Var<Context>, Callback<Error, uint8_t, std::string> cb) {
    cb(NoError(), TEST_PREPARE, "3010");
}

static void failure(std::string, int,
                    Callback<Error, Continuation<Error, double>> cb, double,
                    Settings, Var<Logger>, Var<Reactor>) {
    cb(GenericError(), [](Callback<Error, double>) {
        REQUIRE(false); // should not happen
    });
}

TEST_CASE("run() deals with coroutine failure") {
    Var<Context> ctx(new Context);
    test_s2c::run_impl<success, messages::format_test_msg, messages::read_json,
                       messages::write, failure>(
        ctx, [](Error err) { REQUIRE(err == GenericError()); });
}

static int failure_in_test_start_cnt = 0;
static void failure_in_test_start(Var<Context>,
                                  Callback<Error, uint8_t, std::string> cb) {
    if (failure_in_test_start_cnt++ == 0) {
        cb(NoError(), TEST_PREPARE, "3010");
    } else {
        cb(GenericError(), 0, "");
    }
}

static void
connect_but_fail_later(std::string, int,
                       Callback<Error, Continuation<Error, double>> cb, double,
                       Settings, Var<Logger>, Var<Reactor>) {
    cb(NoError(), [](Callback<Error, double> cb) { cb(GenericError(), 0.0); });
}

TEST_CASE("run() deals with error when reading TEST_START") {
    Var<Context> ctx(new Context);
    test_s2c::run_impl<failure_in_test_start, messages::format_test_msg,
                       messages::read_json, messages::write,
                       connect_but_fail_later>(
        ctx, [](Error err) { REQUIRE(err == GenericError()); });
}

static int not_test_start_cnt = 0;
static void not_test_start(Var<Context>,
                           Callback<Error, uint8_t, std::string> cb) {
    if (not_test_start_cnt++ == 0) {
        cb(NoError(), TEST_PREPARE, "3010");
    } else {
        cb(NoError(), MSG_ERROR, "");
    }
}

TEST_CASE("run() deals with unexpected message instead of TEST_START") {
    Var<Context> ctx(new Context);
    test_s2c::run_impl<not_test_start, messages::format_test_msg,
                       messages::read_json, messages::write,
                       connect_but_fail_later>(
        ctx, [](Error err) { REQUIRE(err == GenericError()); });
}

static int test_prepare_and_start_cnt = 0;
static void test_prepare_and_start(Var<Context>,
                                   Callback<Error, uint8_t, std::string> cb) {
    if (test_prepare_and_start_cnt++ == 0) {
        cb(NoError(), TEST_PREPARE, "3010");
    } else {
        cb(NoError(), TEST_START, "");
        test_prepare_and_start_cnt = 0; // Prepare for next invocation
    }
}

TEST_CASE("run() deals with coroutine terminating with error") {
    Var<Context> ctx(new Context);
    test_s2c::run_impl<test_prepare_and_start, messages::format_test_msg,
                       messages::read_json, messages::write,
                       connect_but_fail_later>(
        ctx, [](Error err) { REQUIRE(err == GenericError()); });
}

static void success(std::string, int,
                    Callback<Error, Continuation<Error, double>> cb, double,
                    Settings, Var<Logger>, Var<Reactor>) {
    cb(NoError(), [](Callback<Error, double> cb) { cb(NoError(), 0.0); });
}

static void failure(Var<Context>, Callback<Error, uint8_t, json> cb) {
    cb(GenericError(), 0, {});
}

TEST_CASE("run() deals with error when reading TEST_MSG") {
    Var<Context> ctx(new Context);
    test_s2c::run_impl<test_prepare_and_start, messages::format_test_msg,
                       failure, messages::write, success>(
        ctx, [](Error err) { REQUIRE(err == GenericError()); });
}

static void invalid_msg(Var<Context>, Callback<Error, uint8_t, json> cb) {
    cb(NoError(), MSG_ERROR, {});
}

TEST_CASE("run() deals with unexpected message instead of TEST_MSG") {
    Var<Context> ctx(new Context);
    test_s2c::run_impl<test_prepare_and_start, messages::format_test_msg,
                       invalid_msg, messages::write, success>(
        ctx, [](Error err) { REQUIRE(err == GenericError()); });
}

static void msg_test_ok(Var<Context>, Callback<Error, uint8_t, json> cb) {
    cb(NoError(), TEST_MSG, {});
}

static ErrorOr<Buffer> failure(std::string) { return GenericError(); }

TEST_CASE("run() deals with format_test_msg() failure") {
    Var<Context> ctx(new Context);
    test_s2c::run_impl<test_prepare_and_start, failure, msg_test_ok,
                       messages::write, success>(
        ctx, [](Error err) { REQUIRE(err == GenericError()); });
}

static void failure(Var<Context>, Buffer, Callback<Error> cb) {
    cb(GenericError());
}

TEST_CASE("run() deals with write() failure") {
    Var<Context> ctx(new Context);
    test_s2c::run_impl<test_prepare_and_start, messages::format_test_msg,
                       msg_test_ok, failure, success>(
        ctx, [](Error err) { REQUIRE(err == GenericError()); });
}
