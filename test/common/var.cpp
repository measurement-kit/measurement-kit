// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "private/ext/catch.hpp"

#include <measurement_kit/common.hpp>

using namespace mk;

class Foo {
 public:
    double elem = 3.14;
    Foo() {}
    Foo(double x) : elem(x) {}
    void mascetti() {}
    virtual ~Foo() noexcept;
};

Foo::~Foo() noexcept {}

class FooBar : public Foo {
  public:
    double elem_child = 6.28;
    FooBar(double y) : elem_child(y) {}
    FooBar() {}
    ~FooBar() noexcept override;
};

FooBar::~FooBar() noexcept {}

// Enters the most glorious of the Sith lords...
class JarJar {
  public:
    double binks = 0.91;
    virtual ~JarJar() noexcept;
};

JarJar::~JarJar() noexcept {}

TEST_CASE("Var raises an exception when the pointee is nullptr") {
    Var<Foo> foo;
    double k;
    REQUIRE_THROWS(k = foo->elem);
    REQUIRE_THROWS(*foo);
    REQUIRE_THROWS(foo.get());
}

TEST_CASE("We can safely assign to Var an empty shared_ptr") {
    Var<Foo> necchi = std::shared_ptr<Foo>{};
    double k;
    REQUIRE_THROWS(k = necchi->elem);
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

TEST_CASE("as() works as expected") {
    SECTION("When upcast is possible and target pointer is null") {
        Var<FooBar> bar;
        REQUIRE(!bar);
        Var<Foo> foo = bar.as<Foo>();
        REQUIRE(!foo);
    }

    SECTION("When upcast is possible and target pointer is not null") {
        Var<FooBar> bar{new FooBar};
        REQUIRE(!!bar);
        Var<Foo> foo = bar.as<Foo>();
        REQUIRE(!!foo);
    }

    SECTION("When downcast is possible and target pointer is null") {
        Var<Foo> foo;
        REQUIRE(!foo);
        Var<FooBar> bar = foo.as<FooBar>();
        REQUIRE(!bar);
    }

    SECTION("When downcast is possible and target pointer is not null") {
        Var<Foo> foo{new FooBar};
        REQUIRE(!!foo);
        Var<FooBar> bar = foo.as<FooBar>();
        REQUIRE(!!bar);
    }

    SECTION("When cast is not possible and target pointer is null") {
        Var<Foo> foo;
        REQUIRE(!foo);
        Var<JarJar> jar = foo.as<JarJar>();
        REQUIRE(!jar);
    }

    SECTION("When cast is not possible and target pointer is not null") {
        Var<Foo> foo{new FooBar};
        REQUIRE(!!foo);
        Var<JarJar> jar = foo.as<JarJar>();
        REQUIRE(!jar);
    }

    SECTION("The deleter cannot be preserved") {
        auto count = 0;
        {
            Var<Foo> foo{new FooBar, [&](Foo *p) {
                count += 1;
                delete p;
            }};
            REQUIRE(!!foo);
            Var<FooBar> bar = foo.as<FooBar>();
            REQUIRE(!!bar);
        }
        REQUIRE(count == 1);
    }

    SECTION("The usage count is consistent") {
        Var<FooBar> bar;
        {
            Var<Foo> foo{new FooBar};
            REQUIRE(!!foo);
            bar = foo.as<FooBar>();
            REQUIRE(!!bar);
            REQUIRE(foo.use_count() == 2);
            REQUIRE(bar.use_count() == 2);
        }
        REQUIRE(bar.use_count() == 1);
    }
}
