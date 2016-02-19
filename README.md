# MeasurementKit

[![Build Status](https://travis-ci.org/measurement-kit/measurement-kit.svg?branch=master)](https://travis-ci.org/measurement-kit/measurement-kit)
[![Coverage Status](https://coveralls.io/repos/measurement-kit/measurement-kit/badge.svg?branch=master&service=github)](https://coveralls.io/github/measurement-kit/measurement-kit?branch=master)

MeasurementKit is a library that implements open network measurement methodologies
(performance, censorship, etc.) and targets mobile platforms (Android and iOS).

It is meant to be embedeed by third party applications with specific network measurement
needs and/or to be used by researchers as a basis to implement novel tools.

Currently it implements the following high-level tests:

- [OONI](https://ooni.torproject.org/)'s [DNS Injection](https://github.com/TheTorProject/ooni-spec/blob/master/test-specs/ts-012-dns-injection.md) test

- [OONI](https://ooni.torproject.org/)'s [HTTP Invalid Request Line](https://github.com/TheTorProject/ooni-spec/blob/master/test-specs/ts-007-http-invalid-request-line.md) test

- [OONI](https://ooni.torproject.org/)'s [TCP Connect](https://github.com/TheTorProject/ooni-spec/blob/master/test-specs/ts-008-tcpconnect.md) test

It contains building-block functionalities useful to implement your own
tests. More in detail it currently implements:

- [TCP connection](https://github.com/measurement-kit/measurement-kit/blob/master/include/measurement_kit/net/transport.hpp) (with which you can create a TCP connection towards and
  endpoint, receive and send data)

- [DNS client](https://github.com/measurement-kit/measurement-kit/blob/master/include/measurement_kit/dns/dns.hpp) (with which you can resolve and reverse-resolve A and AAAA
  records using arbitrary name servers)

- [HTTP client](https://github.com/measurement-kit/measurement-kit/blob/master/include/measurement_kit/http.hpp) (with which you can send HTTP/1.1 requests and receive
  and parse the corresponding responses)

- [traceroute for Android](https://github.com/measurement-kit/measurement-kit/blob/master/include/measurement_kit/traceroute/android.hpp) (with which you can send individual traceroute
  probes with a specified payload and TTL)

- [mlab-ns client](https://github.com/measurement-kit/measurement-kit/blob/master/include/measurement_kit/mlabns.hpp) (with which you can interact with Measurement Lab backend to know the server with which to run tests)

In the short term we plan to add to MeasurementKit:

- the [network diagnostic tool](https://github.com/ndt-project/ndt/wiki/NDTTestMethodology) network performance test

- the functionality to communicate with the [OONI backend](https://github.com/TheTorProject/ooni-backend)

- more OONI tests

Other functionalities that we would like to add are building-blocks functionalities
such as [uTP](https://github.com/bittorrent/libutp), and traceroute for iOS.

The following index illustrates the content of the remainder of this file:

- [How to clone the repositorty](#how-to-clone-the-repository)
- [How to test a specific branch](#how-to-test-a-specific-branch)
- [How to build MeasurementKit](#how-to-build-measurementkit)
  - [How to build MeasurementKit on a Unix-like system](#how-to-build-measurementkit-on-a-unix-like-system)
  - [How to test MeasurementKit on a Unix-like system](#how-to-test-measurementkit-on-a-unix-like-system)
  - [How to build MeasurementKit on Android](#how-to-build-measurementkit-on-android)
  - [How to build MeasurementKit on iOS](#how-to-build-measurementkit-on-ios)
  - [How to add MeasurementKit to an Xcode project](#how-to-add-measurementkit-to-an-xcode-project)
- [How to use MeasurementKit](#how-to-use-measurementkit)


## How to clone the repository

To clone MeasurementKit repository, do:

    git clone https://github.com/measurement-kit/measurement-kit

## How to test a specific branch

If you need to checkout a specific branch (say `feature/foo`) for testing
it, clone the repository and then type:

```
git fetch origin
git checkout feature/foo
```

Then proceed with the instruction to build and test MeasurementKit.

## How to build MeasurementKit

### How to build MeasurementKit on a Unix-like system

To build, MeasurementKit needs:

- a C90 compiler (such as gcc or clang)
- a C++11 compiler (such as g++ or clang++)
- autoconf, automake, and libtool
- a Unix environment (such as Linux or MacOS)

C++11 must be enabled, otherwise certain C++11 features such as
`std::function` will not be recognized.

MeasurementKit includes and unconditionally compiles the
sources of the following projects:

- [http-parser](https://github.com/joyent/http-parser)
- [Catch](https://github.com/philsquared/Catch) (for tests only)
- OpenBSD's [strtonum.c](http://cvsweb.openbsd.org/cgi-bin/cvsweb/src/lib/libc/stdlib/strtonum.c)

MeasurementKit also depends on the following projects (which
are only conditionally compiled as explained below):

- [libevent](https://github.com/libevent/libevent)
- [yaml-cpp](https://github.com/jbeder/yaml-cpp)
- selected [boost](https://github.com/boostorg/) libraries (only [the ones required by yaml-cpp](https://github.com/measurement-kit/measurement-kit/tree/master/src/ext/boost))
- [jansson](https://github.com/akheron/jansson)
- [libmaxminddb](https://github.com/maxmind/libmaxminddb)

The `./configure` script should check whether all
the dependencies are in place and should configure the compilers
properly. If a dependency is not found, `./configure` will
fall back to the copy of the dependency stored under the
`src/ext` directory.

The vanilla build process is the following:

    ./autogen.sh
    ./configure
    make

You can also force `./configure` to select dependencies available
at specific directories using the following flags:

- `--with-libevent=PREFIX` that tells `./configure` to use the
libevent library and headers installed at PREFIX

- `--with-yaml-cpp=PREFIX` that tells `./configure` to use the
yaml-cpp library and headers installed at PREFIX

- `--with-boost=PREFIX` that tells `./configure` to use the
boost headers installed at PREFIX

- `--with-jansson=PREFIX` that tells `./configure` to use the
jansson library and headers installed at PREFIX

- `--with-libmaxminddb=PREFIX` that tells `./configure` to use the
libmaxminddb library and headers installed at PREFIX

In all the above cases you can also specify PREFIX equal to
`builtin` to force `./configure` to use builtin sources.

For example,

- if libevent is installed at `/opt/local` (meaning that `event.h`
is `/opt/local/include/event.h` and that `libevent.a` is
`/opt/local/lib/libevent.a`), use

```
    ./configure --with-libevent=/opt/local
```

- to force-compile the libevent distributed along with MeasurementKit, use

```
    ./configure --with-libevent=builtin
```

### How to test MeasurementKit on a Unix-like system

Once you have built MeasurementKit, to compile and run the unit
test programs, run:

    make check

To tell make to produce less output (as in the Linux kernel
build process) run:

    make V=0

### How to build MeasurementKit on Android

To compile MeasurementKit for Android, see the README.md file contained in
the `mobile/android` directory of this repository.

### How to build MeasurementKit on iOS

To compile and use MeasurementKit for iOS, do the following on a MacOSX
system where Xcode and its command line tools have been installed:

```
./mobile/ios/scripts/build.sh
```

### How to add MeasurementKit to an Xcode project.

The CocoaPods podspec has not been submitted yet, but you can already use
it in your project adding this line in your Podfile:

    pod 'measurement_kit',
      :git => 'https://github.com/measurement-kit/measurement-kit.git'

You can use a specific branch, e.g.:

    pod 'measurement_kit',
      :git => 'https://github.com/measurement-kit/measurement-kit.git'
      :branch => 'branch-name'

Similarly, you can use a specific tag, e.g.:

    pod 'measurement_kit', 
      :git => 'https://github.com/measurement-kit/measurement-kit.git'
      :tag => 'v0.x.y'

Then type `pod install` and open `.xcworkspace` file (beware not to open the
`.xcodeproj` file instead, because that alone won't compile).

## How to use MeasurementKit

The following examples show how to use OONI library.

This first example show how to run a synchronous test. That is, in the
following example, the *run* call is going to block until the test is
complete. (Note that in this case the test may or may not run in the context
of the same thread that called *run*).

```C++
#include <measurement_kit/ooni.hpp>

// Run sync test
mk::ooni::HttpInvalidRequestLineTest()
    .set_backend("http://127.0.0.1/")
    .set_verbose()
    .on_log([](const char *s) {
        // If needed, acquire the proper locks
        // Process incoming log line
    })
    .run();

// Note: run() returns when test is complete
```

In this second example, instead, we show how to run an asynchronous test. In
this case, *run* returns immediately, the test runs in a background thread, and
the callback passed as argument to *run* is invoked when the test completed.

```C++
// Run async test
mk::ooni::HttpInvalidRequestLineTest()
    .set_backend("http://127.0.0.1/")
    .set_verbose()
    .on_log([](const char *s) {
        // If needed, acquire the proper locks
        // Process incoming log line
    })
    .run([]() {
        // If needed, acquire the proper locks
        // Handle test completion
    });

// Note: run() returns immediately, callback called when done
```

In both cases, you need to be careful inside the callbacks, because in general
they may be called from background threads.

You can find documentation of MeasurementKit C++ API in the
[doc/api](https://github.com/measurement-kit/measurement-kit/tree/master/doc/api)
folder of the repository.
