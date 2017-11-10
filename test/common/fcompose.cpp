// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "private/ext/catch.hpp"

#include "private/common/fcompose.hpp"

#include <measurement_kit/common.hpp>

using namespace mk;

TEST_CASE("fcompose() works as expected with sync policy") {
    SECTION("For a single function") {
        REQUIRE(fcompose(fcompose_policy_sync(),
                         [](int x) { return x + 2; })(1) == 3);
    }

    SECTION("For two functions") {
        REQUIRE(fcompose(fcompose_policy_sync(), [](int x) { return x + 2; },
                         [](int x) { return x + 4; })(1) == 7);
    }

    SECTION("For three functions") {
        REQUIRE(fcompose(fcompose_policy_sync(), [](int x) { return x + 2; },
                         [](int x) { return x + 4; },
                         [](int x) { return x + 4; })(1) == 11);
    }

    SECTION("For multiple arguments") {
        REQUIRE(fcompose(fcompose_policy_sync(),
                         [](int x) { return std::make_tuple(x, 2); },
                         [](int x, int y) { return x + y + 4; },
                         [](int x) { return x + 4; })(1) == 11);
    }
}

TEST_CASE("fcompose() works as expected with async policy") {
    SECTION("For a single function") {
        fcompose(fcompose_policy_async(), [](int x, Callback<int> &&cb) {
            cb(x + 2);
        })(1, [](int x) { REQUIRE(x == 3); });
    }

    SECTION("For two functions") {
        fcompose(fcompose_policy_async(),
                 [](int x, Callback<int> &&cb) { cb(x + 2); },
                 [](int x, Callback<int> &&cb) { cb(x + 4); })(
              1, [](int x) { REQUIRE(x == 7); });
    }

    SECTION("For three functions") {
        fcompose(fcompose_policy_async(),
                 [](int x, Callback<int> &&cb) { cb(x + 2); },
                 [](int x, Callback<int> &&cb) { cb(x + 4); },
                 [](int x, Callback<int> &&cb) { cb(x + 4); })(
              1, [](int x) { REQUIRE(x == 11); });
    }

    SECTION("For multiple arguments") {
        fcompose(fcompose_policy_async(),
                 [](int x, Callback<int, int> &&cb) { cb(x, 2); },
                 [](int x, int y, Callback<int> &&cb) { cb(x + y + 4); },
                 [](int x, Callback<int> &&cb) { cb(x + 4); })(
              1, [](int x) { REQUIRE(x == 11); });
    }

    SECTION("For deferred callbacks") {
        // Simulate deferred callbacks using reactor->call_soon()
        auto reactor = Reactor::make();
        auto f = fcompose(
              fcompose_policy_async(),
              [reactor](int x, Callback<int, int> &&cb) {
                  reactor->call_soon(
                        [ x = std::move(x), cb = std::move(cb) ]() {
                            cb(x, 2);
                        });
              },
              [reactor](int x, int y, Callback<int> &&cb) {
                  reactor->call_soon([
                      x = std::move(x), y = std::move(y), cb = std::move(cb)
                  ]() { cb(x + y + 4); });
              },
              [reactor](int x, Callback<int> &&cb) {
                  reactor->call_soon(
                        [ x = std::move(x), cb = std::move(cb) ]() {
                            cb(x + 4);
                        });
              });
        reactor->run_with_initial_event([reactor, f]() mutable {
            f(1, [reactor](int x) {
                REQUIRE(x == 11);
                reactor->stop();
            });
        });
    }
}

// A replacement for `std::string` that cannot be copied
class ncs : public std::string, public NonCopyable {
  public:
    ncs(const char *s) : std::string{s} {}

    ncs &operator=(ncs &&x) {
        std::string::operator=(static_cast<std::string>(x));
        return *this;
    }

    ncs(ncs &&x) { *this = std::move(x); }
};

// If the following code compiles, it means we can use full move semantic
TEST_CASE("We can move arguments using fcompose") {
    SECTION("With the sync policy") {
        fcompose(fcompose_policy_sync(),
                 [](ncs &&s) {
                     s += " some message here";
                     return std::make_tuple(ncs{std::move(s)}, "");
                 },
                 [](ncs &&s, ncs &&x) { std::cout << s << x << "\n"; })(
              ncs{"[!]"});
    }

    SECTION("With the async policy") {
        fcompose(fcompose_policy_async(),
                 [](ncs &&s, Callback<ncs &&, ncs &&> &&cb) {
                     s += " some message here";
                     cb(std::move(s), "");
                 },
                 [](ncs &&s, ncs &&x, Callback<> &&cb) {
                     std::cout << s << x << "\n";
                     cb();
                 })(ncs{"[!]"}, []() {});
    }
}
