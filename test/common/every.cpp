// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "private/ext/catch.hpp"

#include "private/common/utils.hpp"
#include "private/common/every.hpp"

#include <measurement_kit/common.hpp>

using namespace mk;

TEST_CASE("The every() template works") {
    SharedPtr<Reactor> reactor = Reactor::make();

    SECTION("When the delay is negative we get an error") {
        Error final_err;
        reactor->run_with_initial_event([&]() {
            every(-1.0, reactor,
                  [&](Error err) {
                      final_err = err;
                      reactor->stop();
                  },
                  []() { return false; }, []() {});
        });
        REQUIRE(final_err == ValueError());
    }

    SECTION("When the delay is negative the callback is deferred") {
        bool called = false;
        reactor->run_with_initial_event([&]() {
            every(-1.0, reactor,
                  [&](Error /*err*/) {
                      called = true;
                      reactor->stop();
                  },
                  []() { return false; }, []() {});
            REQUIRE(called == false);
        });
        REQUIRE(called == true);
    }

    SECTION("In the common case") {
        double now = 0.0;
        double stop = time_now() + 10.0;
        int count = 0;
        reactor->run_with_initial_event([&]() {
            every(1.0, reactor, [&](Error /*err*/) { reactor->stop(); },
                  [&]() { return time_now() > stop; },
                  [&]() {
                      count += 1;
                      if (now == 0.0) {
                          now = time_now();
                          return;
                      }
                      double cur = time_now();
                      double delta = cur - now;
                      REQUIRE(delta > 0.7);
                      REQUIRE(delta < 1.3);
                      now = cur;
                  });
        });
        REQUIRE(count > 8);
        REQUIRE(count < 12);
    }
}
