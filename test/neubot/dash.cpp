#define CATCH_CONFIG_MAIN

#include "src/ext/Catch/single_include/catch.hpp"
#include "src/net/emitter.hpp"
#include "src/neubot/dash_impl.hpp"
#include <measurement_kit/common.hpp>
#include <measurement_kit/net.hpp>
#include <measurement_kit/neubot.hpp>
#include <measurement_kit/report.hpp>

using namespace mk;
using namespace mk::neubot::dash;

static void fail(Settings, Callback<Error, Var<net::Transport>> cb,
                 Var<Reactor> = Reactor::global(),
                 Var<Logger> = Logger::global()) {
    cb(MockedError(), nullptr);
}

TEST_CASE("Make sure that an error is passed to callback if the client "
          "fails to connect to the server") {
    run_impl<fail>({}, [](Error error, Var<report::Entry>) { REQUIRE(error); },
                   "", Reactor::global(), Logger::global());
}

static void fail_to_send(Var<net::Transport>, Settings, Headers, std::string,
                         Callback<Error> cb) {
    cb(MockedError());
}

TEST_CASE("Make sure that an error is passed to callback if the client "
          "fails to send an HTTP request") {
    Var<net::Transport> emitter(new net::Emitter);
    Var<report::Entry> entry(new report::Entry);

    loop_request<fail_to_send>(
        emitter, 0, [](Error error, Var<report::Entry>) { REQUIRE(error); },
        entry, "", {}, Reactor::global(), Logger::global());
}
