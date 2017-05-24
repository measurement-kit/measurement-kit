// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "../src/libmeasurement_kit/ext/catch.hpp"

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

TEST_CASE("U raises an exception when the pointee is nullptr") {
    U<Foo> foo;
    double k;
    REQUIRE_THROWS(k = foo->elem);
    REQUIRE_THROWS(*foo);
    REQUIRE_THROWS(foo.get());
}

TEST_CASE("We can safely assign to U an empty shared_ptr") {
    U<Foo> necchi = []() { return std::unique_ptr<Foo>{}; }();
    double k;
    REQUIRE_THROWS(k = necchi->elem);
    REQUIRE_THROWS(*necchi);
    REQUIRE_THROWS(necchi.get());
}

TEST_CASE("We can assign to U the result of make_unique") {
    U<Foo> necchi = std::make_unique<Foo>(6.28);
    REQUIRE(necchi->elem == 6.28);
    auto foo = *necchi;
    REQUIRE(foo.elem == 6.28);
}

TEST_CASE("The smart pointer works as expected") {
    auto pnecchi = new Foo(6.28);
    U<Foo> necchi{pnecchi};
    REQUIRE(necchi->elem == 6.28);
    REQUIRE((*necchi).elem == 6.28);
    REQUIRE(necchi.get() == pnecchi);
    REQUIRE(necchi.operator->() == pnecchi);
}

TEST_CASE("Operator->() throws when nullptr") {
    U<Foo> necchi;
    REQUIRE_THROWS(necchi->mascetti());
}

TEST_CASE("Operator->* throws when nullptr") {
    U<Foo> sassaroli;
    REQUIRE_THROWS(*sassaroli);
}

TEST_CASE("Get() throws when nullptr") {
    U<Foo> il_melandri;
    REQUIRE_THROWS(il_melandri.get());
}

TEST_CASE("The maker works as expected") {
    U<Foo> x = U<Foo>::make();
    REQUIRE(!!x);
}
