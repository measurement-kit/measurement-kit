# Unix tutorial

This tutorial explains how to compile, install and use Measurement Kit
on Unix.

## Optional: running tutorial in a vagrant

If you want to run this tutorial in a Vagrant, take advantage of the
[Vagrantfile](../../Vagrantfile) and copy it somewhere on the file
system. Then enter into the directory containing the Vagrantfile and run:

```
vagrant up yakkety
```

To spin up an Ubuntu 16.10 machine. Then run:

```
vagrant ssh yakkety
```

At this point you are inside the Vagrant.

## Getting the sources

To this end, you can either clone the git repository or go straight
into the download page and grab the latest sources.

### Cloning the git repository

To clone the repository, you need git installed. Then run:

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

4. run `autoreconf -i` to initialize the [GNU build system](
   https://en.wikipedia.org/wiki/GNU_build_system) used by
   Measurement Kit

For this step to succeed, you certainly need to have installed:

- the usual, standard Unix tools (grep, sed, etc.)

- wget (for downloading GeoIP)

- gzip (for uncompressing GeoIP)

- autoconf, automake, and libtool (for `autoreconf -i`)

### Downloading the sources archive

Go to the [github releases
page](https://github.com/measurement-kit/measurement-kit/releases) and
download the archive of the latest release. **Note**: you want to download
a file called `measurement-kit-$version.tar.gz` and not a file labelled
by GitHub as "Source code", because the latter is equivalent to performing
a `git clone`, for which you should follow the above instructions.

Once you have downloaded the sources archive, you should verify its
digital signature, extract the sources from it, and optionally
download the [GeoIP](https://www.maxmind.com) databases, as described
below.

#### Verifying the sources archive

In addition to the `measurement-kit-$version.tar.gz` file, you should also
download its PGP signature, `measurement-kit-$version.tar.gz.asc`.

Typically, releases are signed by `Simone Basso <bassosimone@gmail.com>`
with a PGP key having this fingerprint:

```
7388 77AA 6C82 9F26 A431  C5F4 80B6 9127 7733 D95B
```

To verify the PGP signature, you need `gpg` installed. Then, fetch the
aforementioned public key with:

```
gpg --recv-keys 738877AA6C829F26A431C5F480B691277733D95B
```

Finally, verify the digital signature with:

```
gpg --verify measurement-kit-$version.tar.gz.asc
```

The result should indicate that the signature is good.

#### Extracting from the sources archive

Once you have verified the digital signature, uncompress the archive:

```
tar -xzf measurement-kit-$version.tar.gz
```

This will create a directory called `measurement-kit-$version`, in
which you should now enter.

#### Downloading GeoIP databases

If you plan to run regress tests, at this point you should also download
the [GeoIP](https://www.maxmind.com) databases with:

```
./build/get-geoip
```

This script will use wget and gzip to fetch and unpack the latest
GeoIP databases in the current working directory.

## Configure, make, make install

You should now be in the top level directory of the sources
containing an executable script called `./configure`.

The bare-bone procedure to compile and install Measurement Kit
is the following:

```
./configure
make
make install  # typically you need to run this step as root
```

In the following, we describe each step in detail.

### configure

The job of the configure script is to make sure that Measurement
Kit will build on your system.

Specifically, the configure script should check for at least for:

- A compiler suite implementing C90 and C++14; e.g. [clang](
  http://clang.llvm.org/) or [gcc](https://gcc.gnu.org/).

- A C++14 standard library such as [libc++](http://libcxx.llvm.org/)
  or [libstdc++](https://gcc.gnu.org/libstdc++/).

- [libevent](https://github.com/libevent/libevent).

- [geoip](https://github.com/maxmind/geoip-api-c).

- either [openssl](https://github.com/openssl/openssl) or
  (preferred) [libressl](https://github.com/libressl-portable/portable).

We routinely compile and test Measurement Kit on:

- Ubuntu 16.10 Yakkety Yak

- Void Linux

- macOS 10.12 sierra

For cross compiling the library on mobile devices, as of Measurement
Kit v0.4.0 we use:

- libevent 2.0.22 (branch `patches-2.0`)

- geoip 1.6.9

- libressl 2.4.4

To run the configure script, simply type:

```
./configure
```

If a dependency is missing, the script will stop and tell you how
you could install such dependency with the proper `apt-get install`
command (for Debian-like systems) and `brew install` command (for
macOS; note that `brew` in not installed by default on macOS, and
you need to [install it manually](http://brew.sh/)).

In addition to those two options, it will also tell you how you
could take advantage of our cross build system for mobile devices
to compile the dependency yourself (not recommended, better relying
on your distribution package manager).

You can pass options to configure to change its behavior. To see all
available options, run `./configure --help`. These are some of the
most commonly used options:

- `--enable-coverage`: build for running coverage tests

- `--disable-examples`: do not compile examples

- `--disable-binaries`: only build `libmeasurement_kit` and do
   not build the `measurement_kit` executable

- `--disable-integration-tests`: only build unit tests and do not
   build integration tests

- `--disable-traceroute`: do not build code for running traceroute
   scans on Android

- `--with-openssl=PATH`: path where openssl (or libressl) is installed

- `--with-libevent=PATH`: path where libevent is installed

- `--with-geoip=PATH`: path where geoip is installed

- `--prefix=PATH`: path where to install Measurement Kit

On Linux, you do not need to tell configure where dependencies are
installed: typically they are installed in canonical places.

On macOS, in my experience you should at least tell configure where
libevent is installed, i.e.:

```
./configure --with-libevent=/usr/local
```

Note that this tells configure that headers are to be found under
`/usr/local/include` and libraries under `/usr/local/lib`.

Also, the configure script will check if your are using brew and search
for an installed openssl even if that is not available under `/usr/local`.

As regards `--prefix`, passing to configure `--prefix=/foo` means that:

- binaries will be installed under `/foo/bin`

- headers will be installed under `/foo/include`

- libs will be installed under `/foo/lib`

If you are building on Linux and you want to use libc++ rather than
the GNU C++ library, make sure you install `clang`, `libc++` (including
its headers, typically in a `-dev` package), `libc++abi` (including
heders). Then export the following variables *before* running `./configure`:

```
export CXX=clang++
export CC=clang
export CXXFLAGS="-stdlib=libc++"
```

### make

If configure succeeds, Measurement Kit is now configured to build
on your system. To compile, make sure make is installed, and then run:

```
make
```

Then, you may want to run Measurement Kit tests:

```
make check
```

If all tests pass, Measurement Kit should work well on your system.

### make install

As a final step, to install Measurement Kit under `/usr/local` (or
under the directory passed to configure using `--prefix`), you need
to become *root* and type:

```
make install
```

On Linux you may also need to update the dynamic linker with:

```
ldconfig
```

Also this command must be run as root.

**Note** If you compiled dependencies using Measurement Kit own
cross compile tool, `make install` will not install them. In such
case you need to locate the headers and libraries under `./builtin`
and copy them manually under `/usr/local/include` and `/usr/local/lib`
(or, more generally, `$prefix/include` and `$prefix/lib`). Beware
not to copy `*.la` files in this process, because this may prevent
Measurement Kit from working correctly, as the `*.la` files compiled
using our cross compile tool hardcode inside them the `./builtin`
prefix, not `/usr/local` (or `$prefix`).

## Using Measurement Kit

To start using Measurement Kit, you should get yourself familiar
with its [nettests API](../api/nettests.md), which is the most
high level API exported by the library. To this end, we suggest
you to study the [examples using such API](../../example/nettests).

These examples are compiled as part of running `make`, unless you
have instructed configure otherwise.

Anyway, each example should also include, as a comment, the correct
command line to compile it.

Remember that, if you have installed Measurement Kit in a non standard
place with `./configure --prefix=$PREFIX`, you need to pass to the compiler
also these flags:

```
-I$PREFIX/include -L$PREFIX/lib
```
