# MeasurementKit

> Portable C++11 network measurement library

[![Android](https://api.bintray.com/packages/measurement-kit/android/android-libs/images/download.svg)](https://bintray.com/measurement-kit/android/android-libs/_latestVersion) [![GitHub license](https://img.shields.io/badge/License-BSD%202--Clause-orange.svg)](https://raw.githubusercontent.com/measurement-kit/measurement-kit/master/LICENSE) [![Github Releases](https://img.shields.io/github/release/measurement-kit/measurement-kit.svg)](https://github.com/measurement-kit/measurement-kit/releases) [![Github Issues](https://img.shields.io/github/issues/measurement-kit/measurement-kit.svg)](https://github.com/measurement-kit/measurement-kit/issues)

- - -

| branch | travis-ci | coveralls | gitlab-ci | circle-ci|
|--------|-----------|-----------|-----------|----------|
| master | [![Travis Build Status](https://travis-ci.org/measurement-kit/measurement-kit.svg?branch=master)](https://travis-ci.org/measurement-kit/measurement-kit) | [![Coverage Status](https://coveralls.io/repos/github/measurement-kit/measurement-kit/badge.svg?branch=master)](https://coveralls.io/github/measurement-kit/measurement-kit?branch=master) | [![GitLab Build Status](https://gitlab.com/measurement-kit/measurement-kit/badges/master/build.svg)](https://gitlab.com/measurement-kit/measurement-kit/commits/master) | [![CircleCI](https://circleci.com/gh/measurement-kit/measurement-kit.svg?style=svg)](https://circleci.com/gh/measurement-kit/measurement-kit) |
| stable | [![Travis Build Status](https://travis-ci.org/measurement-kit/measurement-kit.svg?branch=stable)](https://travis-ci.org/measurement-kit/measurement-kit?branch=stable) | [![Coverage Status](https://coveralls.io/repos/measurement-kit/measurement-kit/badge.svg)](https://coveralls.io/github/measurement-kit/measurement-kit?branch=stable) | [![GitLab Build Status](https://gitlab.com/measurement-kit/measurement-kit/badges/stable/build.svg)](https://gitlab.com/measurement-kit/measurement-kit/commits/stable) | [![CircleCI](https://circleci.com/gh/measurement-kit/measurement-kit/tree/stable.svg?style=svg)](https://circleci.com/gh/measurement-kit/measurement-kit/tree/stable) |

MeasurementKit is a library that implements open network measurement methodologies
(performance, censorship, etc.) and targets mobile platforms (Android and iOS).

It is meant to be embedded by third party applications with specific network measurement
needs and/or to be used by researchers as a basis to implement novel tools.

Currently it implements the following high-level tests:

- [OONI](https://ooni.torproject.org/)'s [Web Connectivity](https://github.com/TheTorProject/ooni-spec/blob/master/test-specs/ts-017-web-connectivity.md) test

- the [network diagnostic tool](https://github.com/ndt-project/ndt/wiki/NDTTestMethodology) network performance test

- [OONI](https://ooni.torproject.org/)'s [DNS Injection](https://github.com/TheTorProject/ooni-spec/blob/master/test-specs/ts-012-dns-injection.md) test

- [OONI](https://ooni.torproject.org/)'s [HTTP Invalid Request Line](https://github.com/TheTorProject/ooni-spec/blob/master/test-specs/ts-007-http-invalid-request-line.md) test

- [OONI](https://ooni.torproject.org/)'s [TCP Connect](https://github.com/TheTorProject/ooni-spec/blob/master/test-specs/ts-008-tcpconnect.md) test

- [OONI](https://ooni.torproject.org/)'s [Meek Fronted Requests](https://github.com/TheTorProject/ooni-spec/blob/master/test-specs/ts-014-meek-fronted-requests.md) test

- [OONI](https://ooni.torproject.org/)'s [HTTP Header Field Manipulation](https://github.com/TheTorProject/ooni-spec/blob/master/test-specs/ts-006-header-field-manipulation.md) test

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

- [mlab-ns client](https://github.com/measurement-kit/measurement-kit/blob/master/include/measurement_kit/mlabns/mlabns.hpp) (with which you can interact with Measurement Lab backend to know the server with which to run tests)

- the functionality to communicate with the [OONI collector](https://github.com/TheTorProject/ooni-backend)

In the short term we plan to add to MeasurementKit:

- Neubot's [MPEG DASH test](https://github.com/neubot/neubot/tree/master/mod_dash)

- the functionality to communicate with the [OONI bouncer](https://github.com/TheTorProject/ooni-backend)

- more OONI tests

Other functionalities that we would like to add are building-blocks functionalities
such as [uTP](https://github.com/bittorrent/libutp), and traceroute for iOS.

The following index illustrates the content of the remainder of this file:

- [How to clone the repository](#how-to-clone-the-repository)
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

For more detailed instructions see [contributing instructions](
CONTRIBUTING.md).

## How to build MeasurementKit

### How to build MeasurementKit on a Unix-like system

Very briefly, to build from the git repository do:

```
./autogen.sh
./configure
make
```

See [the Unix tutorial](doc/tutorial/unix.md) for more details.


### How to test MeasurementKit on a Unix-like system

Once you have built MeasurementKit, run tests like:

```
make check
```

### How to build MeasurementKit on Android

We have [a specific repository](https://github.com/measurement-kit/android-libs)
for compiling MeasurementKit for Android. You may also want to read the
[documentation explaining how to cross compile MK dependencies for Android](
doc/build/android.md).

### How to build MeasurementKit on iOS

Having Xcode command line tools installed, run:

```
./build/ios/library
```

See the [iOS tutorial](doc/tutorial/ios.md) for more info.

### How to add MeasurementKit to an Xcode project.

Make sure your [Podfile](https://guides.cocoapods.org/syntax/podfile.html)
looks like this:

```ruby
target 'YourTargetNameHere' do
    pod 'measurement_kit',
      :git => 'https://github.com/measurement-kit/measurement-kit.git',
      :branch => 'stable'
end
```

Run `pod install` (or `pod update`) and remember to open the
`.xcworkspace` rather than the `.xcodeproj`. See the [iOS tutorial](
doc/tutorial/ios.md) for more info.

## How to use MeasurementKit

You probably want to start using the [nettests API](doc/api/nettests.md)
that is the high level API for running tests. To this end, see also
the [nettests API examples](example/nettests) and the [Unix
tutorial](doc/tutorial/unix.md).
