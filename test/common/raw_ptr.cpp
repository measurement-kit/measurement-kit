// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#define CATCH_CONFIG_MAIN
#include "private/ext/catch.hpp"

#include <measurement_kit/common.hpp>

using namespace mk;

struct Foobar {
    int foo = 17;
    double bar = 3.14;
};

TEST_CASE("The get() method throws if the underlying pointer is null") {
    RawPtr<Foobar> ptr;
    REQUIRE_THROWS(ptr.get());
}

TEST_CASE("The cast operator throws if the underlying pointer is null") {
    RawPtr<Foobar> ptr;
    REQUIRE_THROWS((Foobar *)ptr);
}

TEST_CASE("The release method does not call the destructor") {
    RawPtr<Foobar> ptr{new Foobar};
    delete ptr.release();
    // Valgrind will tell us if we double free
}

TEST_CASE("The reset method does call the destructor") {
    RawPtr<Foobar> ptr{new Foobar};
    ptr.reset(new Foobar);
    // Valgrind will tell us if we leak
}
