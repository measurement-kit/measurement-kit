// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include <measurement_kit/common.hpp>
#include <measurement_kit/net.hpp>

#include "src/common/check_connectivity.hpp"

using namespace mk;
using namespace mk::net;

TEST_CASE("It is possible to use Transport with a custom poller") {
    // Note: this is how Portolan uses measurement-kit
    if (CheckConnectivity::is_down()) {
        return;
    }
    Poller poller;
    Maybe<Transport> maybe_s = mk::net::connect({
        {"family", "PF_UNSPEC"},
        {"address", "nexa.polito.it"},
        {"port", "22"},
    }, Logger::global(), &poller);
    REQUIRE(static_cast<bool>(maybe_s));
    auto s = maybe_s.as_value();
    s.set_timeout(5);
    auto ok = false;
    s.on_error([&poller](Error) { poller.break_loop(); });
    s.on_connect([&poller, &ok]() {
        poller.break_loop();
        ok = true;
    });
    poller.loop();
    REQUIRE(ok);
}
