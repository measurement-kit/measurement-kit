# Measurement Kit

> Portable C++14 network measurement library

[![Android](https://img.shields.io/bintray/v/measurement-kit/android/android-libs.svg)](https://bintray.com/measurement-kit/android/android-libs/_latestVersion) [![GitHub license](https://img.shields.io/badge/License-BSD%202--Clause-orange.svg)](https://raw.githubusercontent.com/measurement-kit/measurement-kit/master/LICENSE) [![Github Releases](https://img.shields.io/github/release/measurement-kit/measurement-kit.svg)](https://github.com/measurement-kit/measurement-kit/releases) [![Github Issues](https://img.shields.io/github/issues/measurement-kit/measurement-kit.svg)](https://github.com/measurement-kit/measurement-kit/issues)

- - -

| branch | travis-ci | coveralls | appveyor |
|--------|-----------|-----------|----------|
| master | [![Travis Build Status](https://img.shields.io/travis/measurement-kit/measurement-kit/master.svg)](https://travis-ci.org/measurement-kit/measurement-kit) | [![Coverage Status](https://img.shields.io/coveralls/measurement-kit/measurement-kit/master.svg)](https://coveralls.io/github/measurement-kit/measurement-kit?branch=master) | [![Build status](https://ci.appveyor.com/api/projects/status/bvse3kgbp8dpk9ii/branch/master?svg=true)](https://ci.appveyor.com/project/bassosimone/measurement-kit/branch/master) |
| stable | [![Travis Build Status](https://img.shields.io/travis/measurement-kit/measurement-kit/stable.svg)](https://travis-ci.org/measurement-kit/measurement-kit?branch=stable) | [![Coverage Status](https://img.shields.io/coveralls/measurement-kit/measurement-kit/stable.svg)](https://coveralls.io/github/measurement-kit/measurement-kit?branch=stable) | [![Build status](https://ci.appveyor.com/api/projects/status/bvse3kgbp8dpk9ii/branch/stable?svg=true)](https://ci.appveyor.com/project/bassosimone/measurement-kit/branch/stable) |

Measurement Kit is a library that implements open network measurement
methodologies (performance, censorship, etc.) for Android, iOS, Windows,
macOS, and Linux systems.

It is meant to be embedded by third party applications with specific network
measurement needs and/or to be used by researchers as a basis to implement
novel tools.

Please, refer to the [include/measurement_kit](include/measurement_kit)
folder documentation for an up-to-date list of available tests.

## API and examples

Measurement Kit exposes a simple C-like API that is described in detail
in the [include/measurement_kit](include/measurement_kit) folder
documentation. You can find examples of usage of such API into the
[example](example) folder.

## Generic Unix instructions

You need to have install the autotools, a C++14 capable C++ compiler, a C++14
capable C++ library, a C11 capable C compiler, and all the dependencies
installed. For current information, we encourage you to read the very simple
build script that we use on Travis CI to setup a Unix environment, from
which you can gather up-to-date information regarding required packages on
a Debian like system. To this end, please refer to the content of the
[.ci/docker](.ci/docker) folder.

```
./autogen.sh
./configure
make
sudo make install    # optional, if you want to install to `/usr/local`
sudo /sbin/ldconfig  # required only on Linux if you install
```

The `./autogen.sh` script should further provide you with pointers. The
configure script will also provide advice if a dependency is missing. Also,
`./configure --help` provides information on how to configure the build.

## Homebrew instructions

Please check the [measurement-kit/homebrew-measurement-kit](
https://github.com/measurement-kit/homebrew-measurement-kit) tap
for Homebrew.

## Mingw-w64 instructions

For information on how to compile Measurement Kit using the [Mingw-w64](
https://mingw-w64.org/doku.php) compiler distribution for Windows, please refer
to the [measurement-kit/script-build-unix](
https://github.com/measurement-kit/measurement-kit) repository.

To better understand how to compile using Mingw-w64, it is also useful to
look into how we build Measurement Kit on AppVeyor. To this end, we encourage
you check the [.ci/appveyor](.ci/appveyor) folder.

## Android instructions

For cross-compiling Measurement Kit for Android, please refer to the
[measurement-kit/script-build-unix](
https://github.com/measurement-kit/script-build-unix) repository. For
integrating Measurement Kit cross-compiled for Android with Java classes
that you can use from Android, please see [measurement-kit/android-libs](
https://github.com/measurement-kit/android-libs). For how to use Measurement
Kit in an Android project, please see [measurement-kit/android-example](
https://github.com/measurement-kit/android-example).

## iOS instructions

For cross-compiling Measurement Kit for iOS, please refer to the
[measurement-kit/script-build-unix](
https://github.com/measurement-kit/script-build-unix) repository. For
integrating Measurement Kit cross-compiled for iOS into a framework
that you can use from iOS, please see [measurement-kit/ios-libs](
https://github.com/measurement-kit/ios-libs). For how to use Measurement
Kit in an iOS project, please see [measurement-kit/ios-example](
https://github.com/measurement-kit/ios-example).

## How to develop for Measurement Kit

To clone Measurement Kit repository, do:

    git clone https://github.com/measurement-kit/measurement-kit

See [CONTRIBUTING.md](CONTRIBUTING.md) for more information.
