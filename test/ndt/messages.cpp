// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "test/winsock.hpp"

#include "include/private/catch.hpp"

#include "src/libmeasurement_kit/ndt/messages_impl.hpp"

using namespace mk;
using namespace mk::ndt;
using namespace mk::net;

static void fail(SharedPtr<Transport>, SharedPtr<Buffer>, size_t, Callback<Error> cb,
                 SharedPtr<Reactor> = Reactor::make()) {
    cb(MockedError());
}

static void succeed(SharedPtr<Transport>, SharedPtr<Buffer>, size_t, Callback<Error> cb,
                    SharedPtr<Reactor> = Reactor::make()) {
    cb(NoError());
}

TEST_CASE("read_ndt() deals with the first readn() error") {
    SharedPtr<Context> ctx(new Context);
    messages::read_ll_impl<fail>(ctx, [](Error err, uint8_t, std::string) {
        REQUIRE(err == ReadingMessageTypeLengthError());
    }, Reactor::make());
}

TEST_CASE("read_ndt() deals with the second readn() error") {
    SharedPtr<Context> ctx(new Context);
    // Now we have enough bytes to read type and lenght such that we
    // arrive to invoke readn() one second time
    ctx->buff->write_uint8(1);
    ctx->buff->write_uint16(3);
    messages::read_ll_impl<succeed, fail>(
        ctx, [](Error err, uint8_t, std::string) {
            REQUIRE(err == ReadingMessagePayloadError());
        }, Reactor::make());
}

static void fail(SharedPtr<Context>, Callback<Error, uint8_t, std::string> cb,
                 SharedPtr<Reactor> = Reactor::make()) {
    cb(MockedError(), 0, "");
}

static void succeed(SharedPtr<Context>, Callback<Error, uint8_t, std::string> cb,
                    SharedPtr<Reactor> = Reactor::make()) {
    cb(NoError(), 0, "");
}

TEST_CASE("read_json() deals with read_ndt() error") {
    SharedPtr<Context> ctx(new Context);
    messages::read_json_impl<fail>(
        ctx, [](Error err, uint8_t, nlohmann::json) { REQUIRE(err == MockedError()); },
        Reactor::make());
}

TEST_CASE("read_json() deals with invalid JSON") {
    SharedPtr<Context> ctx(new Context);
    messages::read_json_impl<succeed>(ctx, [](Error err, uint8_t, nlohmann::json) {
        REQUIRE(err == JsonProcessingError());
    }, Reactor::make());
}

static void fail(SharedPtr<Context>, Callback<Error, uint8_t, nlohmann::json> cb,
                 SharedPtr<Reactor> = Reactor::make()) {
    cb(MockedError(), 0, {});
}

static void invalid(SharedPtr<Context>, Callback<Error, uint8_t, nlohmann::json> cb,
                    SharedPtr<Reactor> = Reactor::make()) {
    cb(NoError(), 0, {{"foo", "baz"}, {"baz", 1}});
}

TEST_CASE("read() deals with read_json() error") {
    SharedPtr<Context> ctx(new Context);
    messages::read_msg_impl<fail>(ctx, [](Error err, uint8_t, std::string) {
        REQUIRE(err == MockedError());
    }, Reactor::make());
}

TEST_CASE("read() deals with json without 'msg' field") {
    SharedPtr<Context> ctx(new Context);
    messages::read_msg_impl<invalid>(ctx, [](Error err, uint8_t, std::string) {
        REQUIRE(err == JsonProcessingError());
    }, Reactor::make());
}

static void bad_type(SharedPtr<Context>, Callback<Error, uint8_t, nlohmann::json> cb,
                    SharedPtr<Reactor> = Reactor::make()) {
    cb(NoError(), 0, 3.14);
}

TEST_CASE("read() deals with json with 'msg' field of the wrong type") {
    SharedPtr<Context> ctx(new Context);
    messages::read_msg_impl<bad_type>(ctx, [](Error err, uint8_t, std::string) {
        REQUIRE(err == JsonProcessingError());
    }, Reactor::make());
}

TEST_CASE("format_any() deals with too large input") {
    ErrorOr<Buffer> x = messages::format_any(1, std::string(131072, 'x'));
    REQUIRE(!x);
    REQUIRE(x.as_error() == MessageTooLongError());
}
