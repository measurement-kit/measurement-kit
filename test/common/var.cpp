// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "src/ext/Catch/single_include/catch.hpp"

#include <measurement_kit/common.hpp>

using namespace mk;

class Foo {
 public:
    double elem = 3.14;
    Foo() {}
    Foo(double x) : elem(x) {}
    void mascetti() {}
};

class FooChild : public Foo {
 public:
     double elemChild = 6.28;
     FooChild(double y) : elemChild(y) {}
};

TEST_CASE("Var raises an exception when the pointee is nullptr") {
    Var<Foo> foo;
    REQUIRE_THROWS(auto k = foo->elem);
    REQUIRE_THROWS(*foo);
    REQUIRE_THROWS(foo.get());
}

TEST_CASE("We can safely assign to Var an empty shared_ptr") {
    std::shared_ptr<Foo> antani;
    Var<Foo> necchi = antani;
    REQUIRE_THROWS(auto k = necchi->elem);
    REQUIRE_THROWS(*necchi);
    REQUIRE_THROWS(necchi.get());
}

TEST_CASE("We can assign to Var the result of make_shared") {
    Var<Foo> necchi = std::make_shared<Foo>(6.28);
    REQUIRE(necchi->elem == 6.28);
    auto foo = *necchi;
    REQUIRE(foo.elem == 6.28);
}

TEST_CASE("The smart pointer works as expected") {
    auto pnecchi = new Foo(6.28);
    Var<Foo> necchi(pnecchi);
    REQUIRE(necchi->elem == 6.28);
    REQUIRE((*necchi).elem == 6.28);
    REQUIRE(necchi.get() == pnecchi);
    REQUIRE(necchi.operator->() == pnecchi);
}

TEST_CASE("Operator->() throws when nullptr") {
    Var<Foo> necchi;
    REQUIRE_THROWS(necchi->mascetti());
}

TEST_CASE("Operator->* throws when nullptr") {
    Var<Foo> sassaroli;
    REQUIRE_THROWS(*sassaroli);
}

TEST_CASE("Get() throws when nullptr") {
    Var<Foo> il_melandri;
    REQUIRE_THROWS(il_melandri.get());
}

TEST_CASE("as() can downcast an object") {
    Var<Foo> foo(new FooChild(7.0));
    Var<FooChild> fooChild = foo.as<FooChild>();
    REQUIRE(fooChild->elemChild == 7.0);
}
