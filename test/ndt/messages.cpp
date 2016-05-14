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
    cb(GenericError());
}

TEST_CASE("read_ndt() deals with readn() error") {
    Var<Context> ctx(new Context);
    messages::read_ndt_impl<fail>(ctx, [](Error err, uint8_t, std::string) {
        REQUIRE(err == GenericError());
    });
}

static void success(Var<Transport>, Var<Buffer>, size_t, Callback<Error> cb) {
    cb(NoError());
}

TEST_CASE("read_ndt() deals with read_uint8() error") {
    Var<Context> ctx(new Context);
    messages::read_ndt_impl<success>(ctx, [](Error err, uint8_t, std::string) {
        REQUIRE(err == NotEnoughDataError());
    });
}

TEST_CASE("read_ndt() deals with read_uint16() error") {
    Var<Context> ctx(new Context);
    ctx->buff->write_uint8(1);
    ctx->buff->write_uint8(3);
    messages::read_ndt_impl<success>(ctx, [](Error err, uint8_t, std::string) {
        REQUIRE(err == NotEnoughDataError());
    });
}

static void second_fail(Var<Transport>, Var<Buffer>, size_t, Callback<Error> cb) {
    static int count = 0;
    if (count++ == 0) {
        cb(NoError());
    } else {
        cb(GenericError());
    }
}

TEST_CASE("read_ndt() deals with the second readn() error") {
    Var<Context> ctx(new Context);
    ctx->buff->write_uint8(1);
    ctx->buff->write_uint16(3);
    messages::read_ndt_impl<second_fail>(ctx,
                                         [](Error err, uint8_t, std::string) {
        REQUIRE(err == GenericError());
    });
}

static void fail(Var<Context>, Callback<Error, uint8_t, std::string> cb) {
    cb(GenericError(), 0, "");
}

TEST_CASE("read_json() deals with read_ndt() error") {
    Var<Context> ctx(new Context);
    messages::read_json_impl<fail>(ctx, [](Error err, uint8_t, json) {
        REQUIRE(err == GenericError());
    });
}

static void success(Var<Context>, Callback<Error, uint8_t, std::string> cb) {
    cb(NoError(), 0, "");
}

TEST_CASE("read_json() deals with invalid JSON") {
    Var<Context> ctx(new Context);
    messages::read_json_impl<success>(ctx, [](Error err, uint8_t, json) {
        REQUIRE(err == GenericError());
    });
}

static void fail(Var<Context>, Callback<Error, uint8_t, json> cb) {
    cb(GenericError(), 0, {});
}

TEST_CASE("read() deals with read_json() error") {
    Var<Context> ctx(new Context);
    messages::read_impl<fail>(ctx, [](Error err, uint8_t, std::string) {
        REQUIRE(err == GenericError());
    });
}

static void invalid(Var<Context>, Callback<Error, uint8_t, json> cb) {
    cb(NoError(), 0, {});
}

TEST_CASE("read() deals with json without 'msg' field") {
    Var<Context> ctx(new Context);
    messages::read_impl<invalid>(ctx, [](Error err, uint8_t, std::string) {
        REQUIRE(err == GenericError());
    });
}

TEST_CASE("format_any() deals with too large input") {
    ErrorOr<Buffer> x = messages::format_any(1, std::string(131072, 'x'));
    REQUIRE(!x);
    REQUIRE(x.as_error() == ValueError());
}
