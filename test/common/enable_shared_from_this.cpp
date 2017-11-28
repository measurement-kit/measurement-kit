// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "private/ext/catch.hpp"

#include <measurement_kit/common.hpp>

using namespace mk;

class Foo : public EnableSharedFromThis<Foo> {
 public:
    double elem = 3.14;
};

TEST_CASE("EnableSharedFromThis works") {
    SECTION("In the common case") {
        auto pfoo = std::make_shared<Foo>();
        SharedPtr<Foo> foo{pfoo->shared_from_this()};
        // Valgrind will tell us if we're leaking
    }

    SECTION("In the problematic case #1") {
        auto foo = new Foo;
        REQUIRE_THROWS(foo->shared_from_this());
        delete foo;
    }

    SECTION("In the problematic case #2") {
        Foo foo;
        REQUIRE_THROWS(foo.shared_from_this());
    }
}
