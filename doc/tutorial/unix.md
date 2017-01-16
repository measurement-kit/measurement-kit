# Unix tutorial

This tutorial explains how to compile, install and use Measurement Kit
on Unix. To start off, you need to get the sources.

## Getting the sources

To this end, you can either clone the git repository or go straight
into the download page and grab the latest sources.

### Cloning the git repository

To clone the repository using git, run:

```
git clone https://github.com/measurement-kit/measurement-kit
```

The result is a directory called `measurement_kit`. Enter into this
directory and run the following command to prepare the sources for
being compiled:

```
./autogen.sh
```

This script will perform the following operations:

1. update the `.gitignore` file

2. generate a list of files to be compiled in `./include.am`

3. get the latest [GeoIP](https://www.maxmind.com) databases

4. run `autoreconf -i` to prepare the GNU build system (used
   by MeasurementKit) to compile the library

For this step to succeed, you certainly need to have installed the
usual, standard Unix tools, wget (for downloading GeoIP), gzip (for
uncompressing GeoIP), autoconf, automake and libtool.

### Downloading the sources archive

Go to the [github releases
page](https://github.com/measurement-kit/measurement-kit/releases) and
download the archive of the latest release. **Note**: you want to download
a file called `measurement-kit-$version.tar.gz` and not a file labelled
by GitHub as "Source code", because the latter is equivalent to performing
a `git clone`, for which you should follow the above instructions.

Once you have downloaded the sources archive, you should verify its
digital signature, extract the sources from it, and optionally
download the [GeoIP](https://www.maxmind.com) databases. These steps
are described below.

#### Verifying the sources archive

In addition to the `measurement-kit-$version.tar.gz` file, you should also
download its PGP signature, `measurement-kit-$version.tar.gz.asc`.

Typically, releases are signed by `Simone Basso <bassosimone@gmail.com>`
with a PGP key having this fingerprint:

```
7388 77AA 6C82 9F26 A431  C5F4 80B6 9127 7733 D95B
```

To verify the PGP signature, you need `gpg2` installed and to follow
these steps:

```
gpg2 --recv-keys 738877AA6C829F26A431C5F480B691277733D95B
gpg2 --verify measurement-kit-$version.tar.gz.asc
```

The first command receives the key, the second command verifies the
tarball. The result should indicate that the signature is good. If in
doubt, *right after* the second command, run `echo $?` and verify that
the result is `0` (meaning that the second command succeded).

#### Extracting from the sources archive

Once you have verified the digital signature, uncompress the archive:

```
tar -xzf measurement-kit-$version.tar.gz
```

This will create a directory called `measurement-kit-$version`, in
which you should now enter.

#### Downloading GeoIP databases

If you plan to run regress tests, at this point you SHOULD also download
the [GeoIP](https://www.maxmind.com) databases that they use, with:

```
./build/get-geoip
```

This script will use wget and gzip to fetch and unpack the latest
GeoIP databases in the current working directory.

## Configure, make, make install

Our build system uses on the so-called [GNU build
system](https://en.wikipedia.org/wiki/GNU_build_system). If you
followed correctly the previous instructions, you should now be in
the top level directory of the sources and such directory should
contain an executable script called `./configure`.

The bare-bone procedure to compile and install Measurement Kit
is the following:

```
./configure
make
make install  # typically you need to run this step as root
```

In the following, we describe each step in detail.

### configure

The job of the configure script is to make sure that the software
packages required to build Measurement Kit are available on your
system. Specifically, the configure script SHOULD check for at least
the following features (more subtle features could be tested):

- A compiler suite implementing C90 and C++11; e.g. [clang](
  http://clang.llvm.org/) or [gcc](https://gcc.gnu.org/).

- A C++11 standard library such as [libc++](http://libcxx.llvm.org/)
  or [libstdc++](https://gcc.gnu.org/libstdc++/).

- [libevent](https://github.com/libevent/libevent).

- [geoip](https://github.com/maxmind/geoip-api-c).

- either [openssl](https://github.com/openssl/openssl) or
  (preferred) [libressl](https://github.com/libressl-portable/portable).

We routinely compile and test Measurement Kit on Ubuntu 16.10 Yakkety
Yak, macOS 10.12 sierra, and Void Linux current. For cross compiling the
library on mobile devices, as of Measurement Kit v0.4.0 we use libevent
2.0.22 (branch `patches-2.0`), geoip 1.6.9, and libressl 2.4.4.

To run the configure script, simply type:

```
./configure
```

If a dependency is missing, the script will stop and tell you how you
could install such dependency on Debian-like systems and macOS. In
particular, on Debian it suggests the proper `apt-get install` command
and on macOS it suggests the proper `brew install` command. Note that
`brew` is not distributed with macOS and you need to [install it
yourself](http://brew.sh/).

In addition to those two options will also tell you how you could
take advantage of our cross build system we use for mobile devices
to compile the dependency yourself (not recommended, if possible
rely on packages installed from your Unix distribution).

You can pass options to configure to change its behavior. To see all
available options, run `./configure --help`. These are some of the
most commonly used options:

- `--enable-coverage`: build for running coverage tests

- `--disable-examples`: do not compile examples

- `--disable-binaries`: only build `libmeasurement_kit` and do
   not build the `measurement_kit` command line executable

- `--disable-integration-tests`: only build unit tests and do not
   build integration tests

- `--disable-traceroute`: do not build code for running traceroute
   scans on Android

- `--with-openssl=PATH`: path where openssl (or libressl) is installed

- `--with-libevent=PATH`: path where libevent is installed

- `--with-geoip=PATH`: path where geoip is installed

- `--prefix=PATH`: path where to install Measurement Kit

Note that, usually, you do not need to tell configure where dependencies
are installed: typically they are installed in canonical places. Yet,
in case you need to specify a `PATH` where a dependency is installed, such
`PATH` should be the directory that contains the `include` and `lib`
directories in turn containing the headers and the libraries of a specific
dependency. For example, given the following filesystem layout:

```
/usr/local/libevent/2.0.22/include/event.h
/usr/local/libevent/2.0.22/lib/libevent.a
```

you want to run:

```
./configure --with-libevent=/usr/local/libevent/2.0.22
```

Also, note that, if you are using macOS and brew, the configure script
will figure that out and will automatically search for a suitable version
of openssl, even if that is not available at the canonical location
(i.e. `/usr/local`).

Similarly, passing to configure `--prefix=/foo` means that:

- binaries will be installed under `/foo/bin`

- headers will be installed under `/foo/include`

- libs will be installed under `/foo/lib`

### make

If configure succeeds, Measurement Kit is now configured to build
on your system. To compile, run:

```
make V=0
```

The `V=0` enables the silent build process, which is more readable than
the standard super-verbose build process.

If also this step succeeds, you may want to run Measurement Kit tests
to make sure that everything was compiled correctly. To do so, run:

```
make check V=0
```

### make install

As a final step, to install Measurement Kit under `/usr/local` (or
under the directory passed to configure using `--prefix`), you need
to become *root* and type:

```
make install
```

**Note** If you compiled dependencies using Measurement Kit own cross
compile tool, note that this will not install them on your system.

On Linux you may need to update the dynamic linker after you have installed
to `/usr/local`, running the following command as root:

```
ldconfig
```

## Using Measurement Kit

Now that Measurement Kit is installed, we can use it. To this end,
we will write a small C++ program that uses it. Specifically, we
will use the [nettests API](../api/nettests.md), the most high level
API in Measurement Kit that allows us to easily specify what test
to run and how. In particular, we will write a small program for
running the [Multi NDT](../api/nettests/multi_ndt.md) test.

Copy the [source code of this example](../../example/nettests/simple.cpp)
in the filesystem as `main.cpp`. Then, to compile it and link it against
MeasurementKit, run:

```
c++ -Wall -Wextra -pedantic -std=c++11 -c main.cpp -o main
```

To run it, type:

```
./main
```
