# MeasurementKit

[![Build Status](https://travis-ci.org/measurement-kit/measurement-kit.svg?branch=master)](https://travis-ci.org/measurement-kit/measurement-kit)

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

- [HTTP client](https://github.com/measurement-kit/measurement-kit/blob/master/include/measurement_kit/http/http.hpp) (with which you can send HTTP/1.1 requests and receive
  and parse the corresponding responses)

- [traceroute for Android](https://github.com/measurement-kit/measurement-kit/blob/master/include/measurement_kit/traceroute/android.hpp) (with which you can send individual traceroute
  probes with a specified payload and TTL)

In the short term we plan to add to MeasurementKit:

- the [network diagnostic tool](https://github.com/ndt-project/ndt/wiki/NDTTestMethodology) network performance test

- the functionality to communicate with the [OONI backend](https://github.com/TheTorProject/ooni-backend)

- more OONI tests

Other functionalities that we would like to add are building-blocks functionalities
such as [uTP](https://github.com/bittorrent/libutp), and traceroute for iOS.

To compile MeasurementKit for Android, see the README.md file contained in
the `mobile/android` directory of this repository.

To compile and use MeasurementKit for iOS, do the following:

```
cd mobile/ios
./scripts/build.sh
```

This README.md files continues by explaining you how to compile MeasurementKit
on a UNIX or UNIX-like platform (e.g. Linux, MacOS).

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

To build, MeasurementKit needs:

- a C90 compiler (such as gcc or clang)
- a C++11 compiler (such as g++ or clang++)
- autoconf, automake, and libtool
- a Unix environment (such as Linux or MacOS)

C++11 must be enabled, otherwise certain C++11 features such as
`std::function` will not be recognized.

MeasurementKit depends on:

- [libevent](https://github.com/libevent/libevent)
- [http-parser](https://github.com/joyent/http-parser)
- [yaml-cpp](https://github.com/jbeder/yaml-cpp)
- [Catch](https://github.com/philsquared/Catch) (for tests only)
- selected [boost](https://github.com/boostorg/) libraries (only [the ones required by yaml-cpp](https://github.com/measurement-kit/measurement-kit/tree/master/src/ext/boost))
- [jansson](https://github.com/akheron/jansson)
- [libmaxminddb](https://github.com/maxmind/libmaxminddb)

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

- to compile the libevent distributed along with MeasurementKit, use

```
    ./configure --with-libevent=builtin
```

## How to test MeasurementKit

Once you have built MeasurementKit, to compile and run the unit
test programs, run:

    make check

To tell make to produce less output (as in the Linux kernel
build process) run:

    make V=0

## How to add measurement-kit to an Xcode project.

The Cocoapods podspec hasn't been submitted yet, but you can still use
it in your project adding this line in your Podfile:

    pod 'measurement_kit', :git => 'https://github.com/measurement-kit/measurement-kit.git'

You can include another branch with: 

    pod 'measurement_kit', 
      :git => 'https://github.com/measurement-kit/measurement-kit.git'
      :branch => 'branch_name'

Then type `pod install` and open `Libight_iOS.xcworkspace` (and not `Libight_iOS.xcodeproj`)

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
