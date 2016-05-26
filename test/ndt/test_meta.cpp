// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include "src/ndt/test_meta_impl.hpp"
#include "src/net/emitter.hpp"
#include <measurement_kit/ndt.hpp>

using namespace mk;
using namespace mk::ndt;
using namespace mk::net;
using json = nlohmann::json;

static void fail(Var<Context>, Callback<Error, uint8_t, std::string> cb,
                 Var<Reactor> = Reactor::global()) {
    cb(MockedError(), 0, "");
}

TEST_CASE("run() deals with read() error") {
    Var<Context> ctx(new Context);
    test_meta::run_impl<fail>(
        ctx, [](Error err) { REQUIRE(err == ReadingTestPrepareError()); });
}

static void unexpected(Var<Context>, Callback<Error, uint8_t, std::string> cb,
                       Var<Reactor> = Reactor::global()) {
    cb(NoError(), MSG_ERROR, "");
}

TEST_CASE("run() deals with unexpected message type") {
    Var<Context> ctx(new Context);
    test_meta::run_impl<unexpected>(
        ctx, [](Error err) { REQUIRE(err == NotTestPrepareError()); });
}

static void test_prepare(Var<Context>,
                         Callback<Error, uint8_t, std::string> cb,
                         Var<Reactor> = Reactor::global()) {
    cb(NoError(), TEST_PREPARE, "");
}

TEST_CASE("run() deals with second read() error") {
    Var<Context> ctx(new Context);
    test_meta::run_impl<test_prepare, fail>(
        ctx, [](Error err) { REQUIRE(err == ReadingTestStartError()); });
}

TEST_CASE("run() deals with unexpected message on second read") {
    Var<Context> ctx(new Context);
    test_meta::run_impl<test_prepare, unexpected>(
        ctx, [](Error err) { REQUIRE(err == NotTestStartError()); });
}

static ErrorOr<Buffer> fail(std::string) { return MockedError(); }

static ErrorOr<Buffer> success(std::string s) {
    Buffer buff;
    buff.write(s);
    return buff;
}

static void test_start(Var<Context>, Callback<Error, uint8_t, std::string> cb,
                       Var<Reactor> = Reactor::global()) {
    cb(NoError(), TEST_START, "");
}

TEST_CASE("run() deals with first format_test_msg() error") {
    Var<Context> ctx(new Context);
    test_meta::run_impl<test_prepare, test_start, fail>(ctx, [](Error err) {
        REQUIRE(err == SerializingClientVersionError());
    });
}

TEST_CASE("run() deals with second format_test_msg() error") {
    Var<Context> ctx(new Context);
    ctx->txp.reset(new Emitter);
    test_meta::run_impl<test_prepare, test_start, success, fail>(
        ctx,
        [](Error err) { REQUIRE(err == SerializingClientApplicationError()); });
}

TEST_CASE("run() deals with third format_test_msg() error") {
    Var<Context> ctx(new Context);
    ctx->txp.reset(new Emitter);
    test_meta::run_impl<test_prepare, test_start, success, success, fail>(
        ctx, [](Error err) { REQUIRE(err == SerializingFinalMetaError()); });
}

static void fail(Var<Context>, Buffer, Callback<Error> cb) {
    cb(MockedError());
}

TEST_CASE("run() deals with write error") {
    Var<Context> ctx(new Context);
    ctx->txp.reset(new Emitter);
    test_meta::run_impl<test_prepare, test_start, success, success, success,
                        fail>(
        ctx, [](Error err) { REQUIRE(err == WritingMetaError()); });
}

static void success(Var<Context>, Buffer, Callback<Error> cb) { cb(NoError()); }

TEST_CASE("run() deals with third read() error") {
    Var<Context> ctx(new Context);
    ctx->txp.reset(new Emitter);
    test_meta::run_impl<test_prepare, test_start, success, success, success,
                        success, fail>(
        ctx, [](Error err) { REQUIRE(err == ReadingTestFinalizeError()); });
}

TEST_CASE("run() deals with unexpected message on third read") {
    Var<Context> ctx(new Context);
    ctx->txp.reset(new Emitter);
    test_meta::run_impl<test_prepare, test_start, success, success, success,
                        success, unexpected>(
        ctx, [](Error err) { REQUIRE(err == NotTestFinalizeError()); });
}
