// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include "src/ndt/messages_impl.hpp"
#include <measurement_kit/ndt.hpp>

using namespace mk;
using namespace mk::ndt;
using namespace mk::net;
using json = nlohmann::json;

static void fail(Var<Transport>, Var<Buffer>, size_t, Callback<Error> cb) {
    cb(MockedError());
}

static void succeed(Var<Transport>, Var<Buffer>, size_t, Callback<Error> cb) {
    cb(NoError());
}

TEST_CASE("read_ndt() deals with the first readn() error") {
    Var<Context> ctx(new Context);
    messages::read_ndt_impl<fail>(ctx, [](Error err, uint8_t, std::string) {
        REQUIRE(err == ReadingMessageTypeLengthError());
    });
}

TEST_CASE("read_ndt() deals with read_uint8() error") {
    Var<Context> ctx(new Context);
    // Do not write any byte so we cannot read a uint8
    messages::read_ndt_impl<succeed>(ctx, [](Error err, uint8_t, std::string) {
        REQUIRE(err == ReadingMessageTypeError());
    });
}

TEST_CASE("read_ndt() deals with read_uint16() error") {
    Var<Context> ctx(new Context);
    // Write some bytes but not enough to read uint8 plus uint16
    ctx->buff->write_uint8(1);
    ctx->buff->write_uint8(3);
    messages::read_ndt_impl<succeed>(ctx, [](Error err, uint8_t, std::string) {
        REQUIRE(err == ReadingMessageLengthError());
    });
}

TEST_CASE("read_ndt() deals with the second readn() error") {
    Var<Context> ctx(new Context);
    // Now we have enough bytes to read type and lenght such that we
    // arrive to invoke readn() one second time
    ctx->buff->write_uint8(1);
    ctx->buff->write_uint16(3);
    messages::read_ndt_impl<succeed, fail>(ctx, [](Error err, uint8_t, std::string) {
        REQUIRE(err == ReadingMessageBodyError());
    });
}

static void fail(Var<Context>, Callback<Error, uint8_t, std::string> cb) {
    cb(MockedError(), 0, "");
}

static void succeed(Var<Context>, Callback<Error, uint8_t, std::string> cb) {
    cb(NoError(), 0, "");
}

TEST_CASE("read_json() deals with read_ndt() error") {
    Var<Context> ctx(new Context);
    messages::read_json_impl<fail>(ctx, [](Error err, uint8_t, json) {
        REQUIRE(err == MockedError());
    });
}

TEST_CASE("read_json() deals with invalid JSON") {
    Var<Context> ctx(new Context);
    messages::read_json_impl<succeed>(ctx, [](Error err, uint8_t, json) {
        REQUIRE(err == JsonParseError());
    });
}

static void fail(Var<Context>, Callback<Error, uint8_t, json> cb) {
    cb(MockedError(), 0, {});
}

static void invalid(Var<Context>, Callback<Error, uint8_t, json> cb) {
    cb(NoError(), 0, {});
}

TEST_CASE("read() deals with read_json() error") {
    Var<Context> ctx(new Context);
    messages::read_impl<fail>(ctx, [](Error err, uint8_t, std::string) {
        REQUIRE(err == MockedError());
    });
}

TEST_CASE("read() deals with json without 'msg' field") {
    Var<Context> ctx(new Context);
    messages::read_impl<invalid>(ctx, [](Error err, uint8_t, std::string) {
        REQUIRE(err == JsonKeyError());
    });
}

TEST_CASE("format_any() deals with too large input") {
    ErrorOr<Buffer> x = messages::format_any(1, std::string(131072, 'x'));
    REQUIRE(!x);
    REQUIRE(x.as_error() == MessageTooLongError());
}
