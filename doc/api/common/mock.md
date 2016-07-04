# NAME
Mock &mdash; Macros to help mocking functions in regress tests

# LIBRARY
MeasurementKit (libmeasurement_kit, -lmeasurement_kit).

# SYNOPSIS

```C++
#include <measurement_kit/common.hpp>

#define MK_MOCK(name_) decltype(name_) name_ = name_
#define MK_MOCK_SUFFIX(name_, suffix_) decltype(name_) name_##_##suffix_ = name_
define MK_MOCK_NAMESPACE(ns_, name_)                                          \
    decltype(ns_::name_) ns_##_##name_ = ns_::name_
#define MK_MOCK_NAMESPACE_SUFFIX(ns_, name_, suffix_)                          \
    decltype(ns_::name_) ns_##_##name_##_##suffix_ = ns_::name_
```

# DESCRIPTION

The `MK_MOCK` macro simplifies life when you use templates for mocking APIs because
allows you to write the following:

```C++
    template <MK_MOCK(event_base_new)>
    void foobar() {
        // Here `event_base_new` is the template parameter which by default
        // points to the `event_base_new` function but can be overriden
        event_base *p = event_base_new();
    }
```

which is arguably faster than writing the following:

```C++
    template <decltype(event_base_new) event_base_new = ::event_base_new>
    void foobar() {
      ...
```

`MK_MOCK_SUFFIX` is similar to `MOCK` but also takes a suffix that
is appended to the function name within the template. This is useful
when you want to distinguish the N-th and the N+1-th invocation of
a function within another function.

The `MK_MOCK_NAMESPACE` and `MK_MOCK_NAMESPACE_SUFFIX` macros are
equal to `MK_MOCK` and `MK_MOCK_SUFFIX` respectively, except that you
get also to specify a namespace and that such namespace plus underscore
are prefixed to the function name. For example:

```C++
  namespace foo {
  void bar();
  }

  template<MK_MOCK_SUFFIX(foo, bar, 123)> void func() {
    foo_bar_123();
  }
```

# EXAMPLE

Usage of these macros is typically coupled with a pattern used in MK
by which implementation of a functionality consists of:

1) a public header file containing prototypes

```C++
// include/measurement_kit/foo/foo.hpp
#ifndef MEASUREMENT_KIT_FOO_FOO_HPP
#define MEASUREMENT_KIT_FOO_FOO_HPP

namespace foo {
void bar();
}

#endif
```

2) a private implementation header containing templates

```C++
// src/foo/foo_impl.hpp
#ifndef SRC_FOO_IMPL_HPP
#define SRC_FOO_IMPL_HPP

#include <measurement_kit/foo.h>
#include <measurement_kit/foobar.h>

namespace foo {

template<MK_MOCK_NAMESPACE_SUFFIX(foobar, bbbaz, 1),
         MK_MOCK_NAMESPACE_SUFFIX(foobar, bbbaz, 2)>
void bar_impl() {
    // With default arguments is equal to calling foobar::bbbaz()
    foobar_bbbaaz_1([=](Error err) {
        if (err) {
            throw err;
        }
        // With default arguments is equal to calling foobar::bbbaz()
        foobar_bbbaz_2([=](Error err) {
            if (err) {
                mk::warn("something's wrong");
            }
        });
    });
}

}
#endif
```

3) a private implementation file pulling the private header

```C++
// src/foo/foo.cpp
#include "src/foo/foo_impl.hpp"

void bar() {
    // This way the template using the default arguments is synthetized here
    // and not every time the header is pulled
    bar_impl();
}
```

4) a test source file that overrides the template defaults

```C++
// test/foo/foo.cpp
#include <catch.hpp>
#include "src/foo/foo_impl.hpp"

static void fail(Callback<Error> cb) {
    cb(MockedError());
}

TEST_CASE("Foo deals with initial error") {
    // This overrides the first invocation of foobar::bbbaz with
    // a mocked function that always fail execution
    REQUIRE_THROWS(foo::bar_impl<fail>());
}

static void success(Callback<Error> cb) {
    cb(NoError());
}

TEST_CASE("Foo deals with second error") {
    // In this case we mock success of the first invocation
    // and failure of the second invocation
    foo::bar_impl<success, fail>();
}
```

# HISTORY

The `Mock` macros appeared in MeasurementKit 0.2.0.
