# MeasurementKit

[![Build Status](https://travis-ci.org/measurement-kit/measurement-kit.svg?branch=master)](https://travis-ci.org/measurement-kit/measurement-kit)

MeasurementKit is an experimental library that provides common functionalities
useful to implement open measurement tools on mobile platforms.

## How to clone the repository

To properly clone MeasurementKit repository, make sure that you specify the
`--recursive` command line flag, as in:

    git clone --recursive https://github.com/measurement-kit/measurement-kit

Such flag tells git to clone not only the MeasurementKit repository, but also
the [submodules](http://git-scm.com/docs/git-submodule) contained therein.

Alternatively, once you have cloned the repository, you can checkout all
the submodules using:

    git submodule update --init

## How to build MeasurementKit

To build the library you need a C90 compiler, a C++11 compiler&mdash;C++11 must
be enabled, otherwise certain C++11 features such as `std::function` will
not be recognized&mdash;and a Unix environment.

MeasurementKit depends on:

- [libevent](https://github.com/libevent/libevent)
- [http-parser](https://github.com/joyent/http-parser)
- [yaml-cpp](https://github.com/jbeder/yaml-cpp)
- [Catch](https://github.com/philsquared/Catch) (for tests only)
- selected [boost](https://github.com/boostorg/) libraries (only [the ones required by yaml-cpp](https://github.com/measurement-kit/measurement-kit-deps/tree/master/boost))

The `./configure` script should check whether all
the dependencies are in place and should configure the compilers
properly. If a dependency is not found, `./configure` will
fall back to the copy of the dependency stored under the
`src/ext` directory.

The vanilla build process is the following:

    autoreconf -i
    ./configure
    make

You can also force `./configure` to select dependencies available
at specific directories using the following flags:

- `--with-libevent=PREFIX` that tells `./configure` to use the
libevent library and headers installed at PREFIX

- `--with-libyaml-cpp=PREFIX` that tells `./configure` to use the
yaml-cpp library and headers installed at PREFIX

- `--with-libboost=PREFIX` that tells `./configure` to use the
boost headers installed at PREFIX

In all the above cases you can also specify PREFIX equal to
`builtin` to force `./configure` to use builtin sources.

For example,

- if libevent is installed at `/opt/local` (meaning that `event.h`
is `/opt/local/include/event.h` and that `libevent.a` is
`/opt/local/lib/libevent.a`), use

```
    ./configure --with-libevent=/opt/local
```

- to compile the libevent distributed along with MeasurementKit, use

```
    ./configure --with-libevent=builtin
```

To compile the unit test programs, run:

    make check

To tell make to produce less output (as in the Linux kernel
build process) run:

    make V=0

## How to use MeasurementKit

The following shows how to use MeasurementKit's OONI library.

```C++
#include <measurement_kit/common.hpp>
#include <measurement_kit/ooni.hpp>

#include <iostream>

using namespace measurement_kit::common;
using namespace measurement_kit::ooni;

int main() {
    volatile bool complete = false;
    Async async;

    async.on_empty([&complete]() {
        std::clog << "All tests complete\n";
        complete = true;
    });

    auto test = SharedPointer<HTTPInvalidRequestLine>(
        new HTTPInvalidRequestLine(Settings{
            {"backend", "http://nexa.polito.it/"}
        }));
    test->set_log_verbose(1);
    test->set_log_function([](const char *s) {
        std::clog << s << "\n";
    });
    async.run_test(test, [](SharedPointer<NetTest> test) {
        std::clog << "Test complete: " << test->identifier() << "\n";
    });

    while (!complete) sleep(1);
}
```
