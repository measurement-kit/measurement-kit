// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "../src/libmeasurement_kit/ext/catch.hpp"

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
                         [](int x) { return x + 4; })(std::move(1)) == 11);
    }

    SECTION("For exceptions") {
        REQUIRE_THROWS(fcompose(fcompose_policy_sync(),
                                [](int) { throw std::exception(); })(1));
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

    SECTION("For exceptions") {
        REQUIRE_THROWS(fcompose(
              fcompose_policy_async(),
              [](int, Callback<int, int> &&) { throw std::exception(); },
              [](int x, int y, Callback<int> &&cb) { cb(x + y); })(1,
                                                                   [](int) {}));
    }
}

TEST_CASE("fcompose() works as expected with async_robust policy") {
    SECTION("For a single function") {
        fcompose(fcompose_policy_async_robust(nullptr),
                 [](int x, Callback<int> cb) { cb(x + 2); })(
              1, [](int x) { REQUIRE(x == 3); });
    }

    SECTION("For two functions") {
        fcompose(fcompose_policy_async_robust(nullptr),
                 [](int x, Callback<int> cb) { cb(x + 2); },
                 [](int x, Callback<int> cb) { cb(x + 4); })(
              1, [](int x) { REQUIRE(x == 7); });
    }

    SECTION("For three functions") {
        fcompose(fcompose_policy_async_robust(nullptr),
                 [](int x, Callback<int> cb) { cb(x + 2); },
                 [](int x, Callback<int> cb) { cb(x + 4); },
                 [](int x, Callback<int> cb) { cb(x + 4); })(
              1, [](int x) { REQUIRE(x == 11); });
    }

    SECTION("For multiple arguments") {
        fcompose(fcompose_policy_async_robust(nullptr),
                 [](int x, Callback<int, int> cb) { cb(x, 2); },
                 [](int x, int y, Callback<int> cb) { cb(x + y + 4); },
                 [](int x, Callback<int> cb) { cb(x + 4); })(
              1, [](int x) { REQUIRE(x == 11); });
    }

    SECTION("For exceptions") {
        auto fired = false;
        fcompose(fcompose_policy_async_robust(
                       [&](const std::exception &) { fired = true; }),
                 [](int, Callback<int, int>) { throw std::exception(); },
                 [](int x, int y, Callback<int> cb) { cb(x + y); })(1,
                                                                    [](int) {});
        REQUIRE(!!fired);
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
                     return ncs{std::move(s)};
                 },
                 [](ncs &&s) { std::cout << s << "\n"; })(ncs{"[!]"});
    }

    SECTION("With the async policy") {
        fcompose(fcompose_policy_async(),
                 [](ncs &&s, Callback<ncs &&> &&cb) {
                     s += " some message here";
                     cb(std::move(s));
                 },
                 [](ncs &&s, Callback<> &&cb) {
                     std::cout << s << "\n";
                     cb();
                 })(ncs{"[!]"}, []() {});
    }

    SECTION("With the async_robust policy") {
        fcompose(fcompose_policy_async_robust(nullptr),
                 [](ncs &&s, Callback<ncs &&> &&cb) {
                     s += " some message here";
                     cb(std::move(s));
                 },
                 [](ncs &&s, Callback<> &&cb) {
                     std::cout << s << "\n";
                     cb();
                 })(ncs{"[!]"}, []() {});
    }
}