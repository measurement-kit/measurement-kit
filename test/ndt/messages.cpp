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

static void fail(Var<Transport>, Var<Buffer>, size_t, Callback<Error> cb,
                 Var<Reactor> = Reactor::global()) {
    cb(MockedError());
}

static void succeed(Var<Transport>, Var<Buffer>, size_t, Callback<Error> cb,
                    Var<Reactor> = Reactor::global()) {
    cb(NoError());
}

TEST_CASE("read_ndt() deals with the first readn() error") {
    Var<Context> ctx(new Context);
    messages::read_ll_impl<fail>(ctx, [](Error err, uint8_t, std::string) {
        REQUIRE(err == ReadingMessageTypeLengthError());
    }, Reactor::global());
}

TEST_CASE("read_ndt() deals with the second readn() error") {
    Var<Context> ctx(new Context);
    // Now we have enough bytes to read type and lenght such that we
    // arrive to invoke readn() one second time
    ctx->buff->write_uint8(1);
    ctx->buff->write_uint16(3);
    messages::read_ll_impl<succeed, fail>(
        ctx, [](Error err, uint8_t, std::string) {
            REQUIRE(err == ReadingMessagePayloadError());
        }, Reactor::global());
}

static void fail(Var<Context>, Callback<Error, uint8_t, std::string> cb,
                 Var<Reactor> = Reactor::global()) {
    cb(MockedError(), 0, "");
}

static void succeed(Var<Context>, Callback<Error, uint8_t, std::string> cb,
                    Var<Reactor> = Reactor::global()) {
    cb(NoError(), 0, "");
}

TEST_CASE("read_json() deals with read_ndt() error") {
    Var<Context> ctx(new Context);
    messages::read_json_impl<fail>(
        ctx, [](Error err, uint8_t, json) { REQUIRE(err == MockedError()); },
        Reactor::global());
}

TEST_CASE("read_json() deals with invalid JSON") {
    Var<Context> ctx(new Context);
    messages::read_json_impl<succeed>(ctx, [](Error err, uint8_t, json) {
        REQUIRE(err == JsonParseError());
    }, Reactor::global());
}

static void fail(Var<Context>, Callback<Error, uint8_t, json> cb,
                 Var<Reactor> = Reactor::global()) {
    cb(MockedError(), 0, {});
}

static void invalid(Var<Context>, Callback<Error, uint8_t, json> cb,
                    Var<Reactor> = Reactor::global()) {
    cb(NoError(), 0, {{"foo", "baz"}, {"baz", 1}});
}

TEST_CASE("read() deals with read_json() error") {
    Var<Context> ctx(new Context);
    messages::read_msg_impl<fail>(ctx, [](Error err, uint8_t, std::string) {
        REQUIRE(err == MockedError());
    }, Reactor::global());
}

TEST_CASE("read() deals with json without 'msg' field") {
    Var<Context> ctx(new Context);
    messages::read_msg_impl<invalid>(ctx, [](Error err, uint8_t, std::string) {
        REQUIRE(err == JsonKeyError());
    }, Reactor::global());
}

static void bad_type(Var<Context>, Callback<Error, uint8_t, json> cb,
                    Var<Reactor> = Reactor::global()) {
    cb(NoError(), 0, 3.14);
}

TEST_CASE("read() deals with json with 'msg' field of the wrong type") {
    Var<Context> ctx(new Context);
    messages::read_msg_impl<bad_type>(ctx, [](Error err, uint8_t, std::string) {
        REQUIRE(err == JsonDomainError());
    }, Reactor::global());
}

TEST_CASE("format_any() deals with too large input") {
    ErrorOr<Buffer> x = messages::format_any(1, std::string(131072, 'x'));
    REQUIRE(!x);
    REQUIRE(x.as_error() == MessageTooLongError());
}
