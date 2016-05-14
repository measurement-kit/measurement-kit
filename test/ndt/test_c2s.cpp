// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include "src/ndt/test_c2s_impl.hpp"
#include <measurement_kit/ndt.hpp>

using namespace mk;
using namespace mk::ndt;
using json = nlohmann::json;

static void failure(std::string, int, Callback<Error, Var<Transport>> cb,
                    Settings, Var<Logger>, Var<Reactor>) {
    cb(GenericError(), nullptr);
}

TEST_CASE("coroutine() is robust to connect error") {
    test_c2s::coroutine_impl<failure>(
        "www.google.com", 3301, 10.0,
        [](Error err, Continuation<Error>) { REQUIRE(err == GenericError()); },
        2.0, {}, Logger::global(), Reactor::global());
}

static void failure(Var<Context>, Callback<Error, uint8_t, std::string> cb) {
    cb(GenericError(), 0, "");
}

static void invalid(Var<Context>, Callback<Error, uint8_t, std::string> cb) {
    cb(NoError(), MSG_ERROR, "");
}

TEST_CASE("run() deals with messages::read() failure") {
    Var<Context> ctx(new Context);
    test_c2s::run_impl<failure>(
        ctx, [](Error err) { REQUIRE(err == GenericError()); });
}

TEST_CASE("run() deals with receiving message different from PREPARE") {
    Var<Context> ctx(new Context);
    test_c2s::run_impl<invalid>(
        ctx, [](Error err) { REQUIRE(err == GenericError()); });
}

static void invalid_port(Var<Context>,
                         Callback<Error, uint8_t, std::string> cb) {
    cb(NoError(), TEST_PREPARE, "foobar");
}

TEST_CASE("run() deals with receiving invalid port") {
    Var<Context> ctx(new Context);
    test_c2s::run_impl<invalid_port>(
        ctx, [](Error err) { REQUIRE(err == GenericError()); });
}

static void too_large(Var<Context>, Callback<Error, uint8_t, std::string> cb) {
    cb(NoError(), TEST_PREPARE, "65536");
}

TEST_CASE("run() deals with receiving too large port") {
    Var<Context> ctx(new Context);
    test_c2s::run_impl<too_large>(
        ctx, [](Error err) { REQUIRE(err == GenericError()); });
}

static void too_small(Var<Context>, Callback<Error, uint8_t, std::string> cb) {
    cb(NoError(), TEST_PREPARE, "-1");
}

TEST_CASE("run() deals with receiving too small port") {
    Var<Context> ctx(new Context);
    test_c2s::run_impl<too_small>(
        ctx, [](Error err) { REQUIRE(err == GenericError()); });
}

static void success(Var<Context>, Callback<Error, uint8_t, std::string> cb) {
    cb(NoError(), TEST_PREPARE, "3010");
}

static void failure(std::string, int, double,
                    Callback<Error, Continuation<Error>> cb, double, Settings,
                    Var<Logger>, Var<Reactor>) {
    cb(GenericError(), [](Callback<Error>) {
        REQUIRE(false); // should not happen
    });
}

TEST_CASE("run() deals with coroutine failure") {
    Var<Context> ctx(new Context);
    test_c2s::run_impl<success, failure>(
        ctx, [](Error err) { REQUIRE(err == GenericError()); });
}

static void failure_in_test_start(Var<Context>,
                                  Callback<Error, uint8_t, std::string> cb) {
    static int count = 0;
    if (count++ == 0) {
        cb(NoError(), TEST_PREPARE, "3010");
    } else {
        cb(GenericError(), 0, "");
    }
}

static void connect_but_fail_later(std::string, int, double,
                                   Callback<Error, Continuation<Error>> cb,
                                   double, Settings, Var<Logger>,
                                   Var<Reactor>) {
    cb(NoError(), [](Callback<Error> cb) { cb(GenericError()); });
}

TEST_CASE("run() deals with error when reading TEST_START") {
    Var<Context> ctx(new Context);
    test_c2s::run_impl<failure_in_test_start, connect_but_fail_later>(
        ctx, [](Error err) { REQUIRE(err == GenericError()); });
}

static void not_test_start(Var<Context>,
                           Callback<Error, uint8_t, std::string> cb) {
    static int count = 0;
    if (count++ == 0) {
        cb(NoError(), TEST_PREPARE, "3010");
    } else {
        cb(NoError(), MSG_ERROR, "");
    }
}

TEST_CASE("run() deals with unexpected message instead of TEST_START") {
    Var<Context> ctx(new Context);
    test_c2s::run_impl<not_test_start, connect_but_fail_later>(
        ctx, [](Error err) { REQUIRE(err == GenericError()); });
}

static void test_prepare_and_start(Var<Context>,
                                   Callback<Error, uint8_t, std::string> cb) {
    static int count = 0;
    if (count++ == 0) {
        cb(NoError(), TEST_PREPARE, "3010");
    } else {
        cb(NoError(), TEST_START, "");
        count = 0; // Prepare for next invocation
    }
}

TEST_CASE("run() deals with coroutine terminating with error") {
    Var<Context> ctx(new Context);
    test_c2s::run_impl<test_prepare_and_start, connect_but_fail_later>(
        ctx, [](Error err) { REQUIRE(err == GenericError()); });
}

static void success(std::string, int, double,
                    Callback<Error, Continuation<Error>> cb, double, Settings,
                    Var<Logger>, Var<Reactor>) {
    cb(NoError(), [](Callback<Error> cb) { cb(NoError()); });
}

static void error_at_test_msg(Var<Context>,
                              Callback<Error, uint8_t, std::string> cb) {
    static int count = 0;
    switch (count++) {
    case 0:
        cb(NoError(), TEST_PREPARE, "3010");
        break;
    case 1:
        cb(NoError(), TEST_START, "");
        break;
    case 2:
        cb(GenericError(), 0, "");
        break;
    default:
        REQUIRE(false);
        /* NOTREACHED */
    }
}

TEST_CASE("run() deals with error when reading TEST_MSG") {
    Var<Context> ctx(new Context);
    test_c2s::run_impl<error_at_test_msg, success>(
        ctx, [](Error err) { REQUIRE(err == GenericError()); });
}

static void not_test_msg(Var<Context>,
                         Callback<Error, uint8_t, std::string> cb) {
    static int count = 0;
    switch (count++) {
    case 0:
        cb(NoError(), TEST_PREPARE, "3010");
        break;
    case 1:
        cb(NoError(), TEST_START, "");
        break;
    case 2:
        cb(NoError(), MSG_ERROR, "");
        break;
    default:
        REQUIRE(false);
        /* NOTREACHED */
    }
}

TEST_CASE("run() deals with unexpected message instead of TEST_MSG") {
    Var<Context> ctx(new Context);
    test_c2s::run_impl<not_test_msg, success>(
        ctx, [](Error err) { REQUIRE(err == GenericError()); });
}

static void error_at_test_finalize(Var<Context>,
                                   Callback<Error, uint8_t, std::string> cb) {
    static int count = 0;
    switch (count++) {
    case 0:
        cb(NoError(), TEST_PREPARE, "3010");
        break;
    case 1:
        cb(NoError(), TEST_START, "");
        break;
    case 2:
        cb(NoError(), TEST_MSG, "");
        break;
    case 3:
        cb(GenericError(), 0, "");
        break;
    default:
        REQUIRE(false);
        /* NOTREACHED */
    }
}

TEST_CASE("run() deals with error when reading TEST_FINALIZE") {
    Var<Context> ctx(new Context);
    test_c2s::run_impl<error_at_test_finalize, success>(
        ctx, [](Error err) { REQUIRE(err == GenericError()); });
}

static void not_test_finalize(Var<Context>,
                              Callback<Error, uint8_t, std::string> cb) {
    static int count = 0;
    switch (count++) {
    case 0:
        cb(NoError(), TEST_PREPARE, "3010");
        break;
    case 1:
        cb(NoError(), TEST_START, "");
        break;
    case 2:
        cb(NoError(), TEST_MSG, "");
        break;
    case 3:
        cb(NoError(), MSG_ERROR, "");
        break;
    default:
        REQUIRE(false);
        /* NOTREACHED */
    }
}

TEST_CASE("run() deals with unexpected message instead of TEST_FINALIZE") {
    Var<Context> ctx(new Context);
    test_c2s::run_impl<not_test_finalize, success>(
        ctx, [](Error err) { REQUIRE(err == GenericError()); });
}
