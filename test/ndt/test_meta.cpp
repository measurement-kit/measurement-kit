// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include "src/ndt/test_meta_impl.hpp"
#include <measurement_kit/ndt.hpp>

using namespace mk;
using namespace mk::ndt;
using namespace mk::net;
using json = nlohmann::json;

static void fail(Var<Context>, Callback<Error, uint8_t, std::string> cb) {
    cb(GenericError(), 0, "");
}

TEST_CASE("run() deals with read() error") {
    Var<Context> ctx(new Context);
    test_meta::run_impl<fail>(ctx, [](Error err) {
        REQUIRE(err == GenericError());
    });
}

static void unexpected(Var<Context>, Callback<Error, uint8_t, std::string> cb) {
    cb(NoError(), MSG_ERROR, "");
}

TEST_CASE("run() deals with unexpected message type") {
    Var<Context> ctx(new Context);
    test_meta::run_impl<unexpected>(ctx, [](Error err) {
        REQUIRE(err == GenericError());
    });
}

static void second_fail(Var<Context>, Callback<Error, uint8_t, std::string> cb) {
    static int count = 0;
    if (count++ == 0) {
        cb(NoError(), TEST_PREPARE, "");
    } else {
        cb(GenericError(), 0, "");
    }
}

TEST_CASE("run() deals with second read() error") {
    Var<Context> ctx(new Context);
    test_meta::run_impl<second_fail>(ctx, [](Error err) {
        REQUIRE(err == GenericError());
    });
}

static void second_unexpected(Var<Context>, Callback<Error, uint8_t, std::string> cb) {
    static int count = 0;
    if (count++ == 0) {
        cb(NoError(), TEST_PREPARE, "");
    } else {
        cb(NoError(), MSG_ERROR, "");
    }
}

TEST_CASE("run() deals with unexpected message on second read") {
    Var<Context> ctx(new Context);
    test_meta::run_impl<second_unexpected>(ctx, [](Error err) {
        REQUIRE(err == GenericError());
    });
}

static void success(Var<Context>, Callback<Error, uint8_t, std::string> cb) {
    static int count = 0;
    if (count++ == 0) {
        cb(NoError(), TEST_PREPARE, "");
    } else {
        cb(NoError(), TEST_START, "");
    }
}

static ErrorOr<Buffer> fail(std::string) {
    return GenericError();
}

TEST_CASE("run() deals with first format_test_msg() error") {
    Var<Context> ctx(new Context);
    test_meta::run_impl<success, fail>(ctx, [](Error err) {
        REQUIRE(err == GenericError());
    });
}

static ErrorOr<Buffer> second_fail(std::string) {
    static int count = 0;
    if (count++ == 0) {
        return Buffer();
    }
    return GenericError();
}

TEST_CASE("run() deals with second format_test_msg() error") {
    Var<Context> ctx(new Context);
    test_meta::run_impl<success, second_fail>(ctx, [](Error err) {
        REQUIRE(err == GenericError());
    });
}

static ErrorOr<Buffer> third_fail(std::string) {
    static int count = 0;
    if (count++ < 2) {
        return Buffer();
    }
    return GenericError();
}

TEST_CASE("run() deals with third format_test_msg() error") {
    Var<Context> ctx(new Context);
    test_meta::run_impl<success, third_fail>(ctx, [](Error err) {
        REQUIRE(err == GenericError());
    });
}

static ErrorOr<Buffer> success(std::string) {
    return Buffer();
}

static void fail(Var<Context>, Buffer, Callback<Error> cb) {
    cb(GenericError());
}

TEST_CASE("run() deals with write error") {
    Var<Context> ctx(new Context);
    test_meta::run_impl<success, success, fail>(ctx, [](Error err) {
        REQUIRE(err == GenericError());
    });
}

static void third_fail(Var<Context>, Callback<Error, uint8_t, std::string> cb) {
    static int count = 0;
    if (count == 0) {
        cb(NoError(), TEST_PREPARE, "");
    } else if (count == 1) {
        cb(NoError(), TEST_START, "");
    } else {
        cb(GenericError(), 0, "");
    }
    ++count;
}

static void success(Var<Context>, Buffer, Callback<Error> cb) {
    cb(NoError());
}

TEST_CASE("run() deals with third read() error") {
    Var<Context> ctx(new Context);
    test_meta::run_impl<third_fail, success, success>(ctx, [](Error err) {
        REQUIRE(err == GenericError());
    });
}

static void third_unexpected(Var<Context>, Callback<Error, uint8_t, std::string> cb) {
    static int count = 0;
    if (count == 0) {
        cb(NoError(), TEST_PREPARE, "");
    } else if (count == 1) {
        cb(NoError(), TEST_START, "");
    } else {
        cb(NoError(), MSG_ERROR, "");
    }
    ++count;
}

TEST_CASE("run() deals with unexpected message on third read") {
    Var<Context> ctx(new Context);
    test_meta::run_impl<third_unexpected, success, success>(ctx, [](Error err) {
        REQUIRE(err == GenericError());
    });
}
