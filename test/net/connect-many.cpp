// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN

#include "src/net/connect-many.hpp"
#include "src/net/emitter.hpp"
#include "src/common/check_connectivity.hpp"
#include "src/ext/Catch/single_include/catch.hpp"
#include <measurement_kit/common.hpp>
using namespace mk;
using namespace mk::net;

/*
             _ _
 _   _ _ __ (_) |_
| | | | '_ \| | __|
| |_| | | | | | |_
 \__,_|_| |_|_|\__|

*/

static void success(std::string, int,
        Callback<Var<Transport>> cb,
        Settings, Logger *logger, Poller *) {
    cb(NoError(), Var<Transport>(new Emitter(logger)));
}

TEST_CASE("net::connect_many() correctly handles net::connect() success") {
    Var<ConnectManyCtx> ctx = connect_many_make("www.google.com", 80, 3,
            [](Error err, std::vector<Var<Transport>> conns) {
                REQUIRE(!err);
                REQUIRE(conns.size() == 3);
            },
            {}, Logger::global(), Poller::global());
    connect_many_<success>(ctx);
}

static void fail(std::string, int,
        Callback<Var<Transport>> cb,
        Settings, Logger *, Poller *) {
    cb(GenericError(), Var<Transport>(nullptr));
}

TEST_CASE("net::connect_many() correctly handles net::connect() failure") {
    Var<ConnectManyCtx> ctx = connect_many_make("www.google.com", 80, 3,
            [](Error err, std::vector<Var<Transport>> conns) {
                REQUIRE(err);
                REQUIRE(conns.size() == 0);
            },
            {}, Logger::global(), Poller::global());
    connect_many_<fail>(ctx);
}

/*
 _       _                       _   _
(_)_ __ | |_ ___  __ _ _ __ __ _| |_(_) ___  _ __
| | '_ \| __/ _ \/ _` | '__/ _` | __| |/ _ \| '_ \
| | | | | ||  __/ (_| | | | (_| | |_| | (_) | | | |
|_|_| |_|\__\___|\__, |_|  \__,_|\__|_|\___/|_| |_|
                 |___/
*/

TEST_CASE("net::connect_many() works as expected") {
    if (CheckConnectivity::is_down()) {
        return;
    }
    loop_with_initial_event([]() {
        connect_many("www.google.com", 80, 3,
                [](Error error, std::vector<Var<Transport>> conns) {
                    REQUIRE(!error);
                    REQUIRE(conns.size() == 3);
                    Var<int> n(new int(0));
                    for (auto conn : conns) {
                        REQUIRE(static_cast<bool>(conn));
                        conn->close([n]() {
                            if (++(*n) >= 3) {
                                break_loop();
                            }
                        });
                    }
                });
    });
}
