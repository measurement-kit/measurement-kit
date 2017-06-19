// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef TEST_COMMON_PTR__HPP
#define TEST_COMMON_PTR__HPP

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

#endif
