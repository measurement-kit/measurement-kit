// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

//
// Tests for src/common/pointer.hpp
//

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <measurement_kit/common.hpp>

using namespace measurement_kit::common;

struct Foo {
    double elem = 3.14;
    Foo() {}
    Foo(double x) : elem(x) {}
};

TEST_CASE("SharedPointer raises an exception when the pointee is nullptr") {
    SharedPointer<Foo> foo;
    REQUIRE_THROWS(auto k = foo->elem);
    REQUIRE_THROWS(*foo);
}

TEST_CASE("We can safely assign to SharedPointer an empty shared_ptr") {
    std::shared_ptr<Foo> antani;
    SharedPointer<Foo> necchi = antani;
    REQUIRE_THROWS(auto k = necchi->elem);
    REQUIRE_THROWS(*necchi);
}

TEST_CASE("We can assign to SharedPointer the result of make_shared") {
    SharedPointer<Foo> necchi = std::make_shared<Foo>(6.28);
    REQUIRE(necchi->elem == 6.28);
    auto foo = *necchi;
    REQUIRE(foo.elem == 6.28);
}
