# Unix tutorial

This tutorial explains how to integrate Measurement Kit into your Unix
application. To start off, you need to get the sources.

## Download Measurement Kit

To this end, you can either clone the git repository or go straight
into the download page and grab the latest sources.

To clone the repository using git, type:

    git clone https://github.com/measurement-kit/measurement-kit

The download page is available here:

    https://github.com/measurement-kit/measurement-kit/releases

If you clone using git, you will find a directory named `measurement-kit`
inside your current directory. Otherwise, you will get a tarball named
after the version number (e.g. `v0.1.0-beta.4.tar.gz`). Once you unpack
that tarball, you will get a directory named `measurement-kit-VERSION`
(e.g. `measurement-kit-0.1.0-beta.4`) inside your current directory.

In either case, enter into this directory to configure the sources and
compile Measurement Kit.

## Configure, make, make install

Measurement Kit build system is based on the so-called [GNU build
system](https://en.wikipedia.org/wiki/GNU_build_system). Specifically,
it uses `autoconf`, `automake`, and `libtool`. Make sure you have those
three packages installed on your system before proceeding.

To compile and install Measurement Kit you need to follow a number of
steps. The first step is to download the sources of dependencies and to
generate the `configure` script. To do that, run this command from the
toplevel directory:

    ./autogen.sh

At this point you are ready to *configure* Measurement Kit to build
on your system. The bare minimum requirements to built it are a C++11
compiler, a C90 compiler, make (not necessarily GNU make), a C++ standard
library, a C library. Measurement Kit is known to work with recent
versions of [clang](http://clang.llvm.org/) and [gcc](https://gcc.gnu.org/),
of [libc++](http://libcxx.llvm.org/) and [libstdc++](
https://gcc.gnu.org/libstdc++/). At the moment of writing this tutorial
we tested Measurement Kit with clang 3.6 and gcc 5.2.

Measurement Kit depends at build time on other pieces of software. At the
moment of writing this tutorial, it depends on:

- [libevent](https://github.com/libevent/libevent)
- [geoip](https://github.com/maxmind/geoip-api-c)

You may want to check the most recent version of [README.md](
https://github.com/measurement-kit/measurement-kit/blob/master/README.md)
to check whether the dependencies changed since this tutorial was
written. If so, please let us know.

The `configure` script will fail if a dependency is missing on the
host system and tell you how you could install it.

To start the `configure` script run:

    ./configure

If this script succeeds, Measurement Kit is now configured to build
on your system. To compile, run:

    make V=0

The `V=0` enables the silent build process, which is more readable than
the standard super-verbose build process.

If also this step succeeds, you may want to run Measurement Kit tests
to make sure that everything was compiled correctly. To do so, run:

    make check V=0

As a final step, to install Measurement Kit under `/usr/local`, you need
to become *root* and type:

    make install

This will install Measurement Kit headers under `/usr/local/include` and
Measurement Kit libraries under `/usr/local/lib`. If bundled dependencies
were compiled, they would be installed under `/usr/local` as well.

On Linux you may need to update the dynamic linker after you have installed
to `/usr/local`, running the following command as root:

    ldconfig

## Using Measurement Kit

Now that Measurement Kit is installed, we can use it. To this end, we
will write a small C++ program that uses Measurement Kit api. Specifically,
we will use the [OONI API](
https://github.com/measurement-kit/measurement-kit/tree/master/doc/api/ooni)
to run OONI's [DNS injection test](
https://github.com/TheTorProject/ooni-spec/blob/master/test-specs/ts-012-dns-injection.md).

To start off, create a file called `main.cpp`. We need to tell the compiler
that we need to use Measurement Kit's OONI API. To do so, you need to add
the following at the top of your file:

```C++
#include <measurement_kit/ooni.hpp>
```

This tells the C++ compiler to import the C++ header containing all the
definitions of Measurement Kit's OONI API.

Then, we need to write a skeleton main program. So, we will have:

```C++
#include <measurement_kit/ooni.hpp>

int main() {}
```

To compile this on your system, use the following command:

    c++ -Wall -std=c++11 -o main main.cpp

If this works, it means that the compiler is a C++11 compiler and that
Measurement Kit headers have been correctly found.

Next, let's write some code inside `main()` to read command line arguments
and act accordingly. Specifically, the user shall be able to specify an
optional backend server address (using the `-b` flag) and one or more files
containing domain names to be resolved using the selected backend (or the
default backend). Also, when invoked with zero arguments (or with incorrect
arguments) the program should print an help message.

```C++
#include <measurement_kit/ooni.hpp>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {
    const char *backend = "8.8.8.1:53";
    const char *progname = argv[0];
    int verbose = MK_LOG_INFO;
    int chr;

    while ((chr = getopt(argc, argv, "b:v")) >= 0) {
        switch (chr) {
        case 'b':
            backend = optarg;
            break;
        case 'v':
            verbose = MK_LOG_DEBUG;
            break;
        default:
            printf("usage: %s [-v] [-b backend] input-file [...]\n", progname);
            exit(1);
        }
    }
    argc -= optind;
    argv += optind;
    if (argc <= 0) {
        printf("usage: %s [-v] [-b backend] input-file [...]\n", progname);
        exit(1);
    }
}
```

Next we want to run OONI DnsInjection test on all the remaining arguments,
using as backend the specified backend, or the default one. We will iterate
over all the remaining command line options and launch an instance of the
DNS Injection test for each file. All these tests will run in parallel and
the program will terminate when all of them are complete. To track the
completion status of tests we will use a `volatile int` variable.

```C++
    volatile int running = 0;
    for (; argc > 0; --argc, ++argv, ++running) {
        mk::ooni::DnsInjection()
            .set_options("backend", backend)
            .set_input_filepath(argv[0])
            .set_verbosity(verbose)
            .run([&running]() { --running; });
    }
```

This is enough to run the test asynchronously. All tests will be run in a
background thread. When the test completes, the C++11 lambda passed to `run()`
is called and decrements the shared `running` variable.

Now, we only need to add code to wait for all tests to complete.

```C++
    while (running > 0) sleep(1);
```

Putting everything together:

```C++
#include <measurement_kit/ooni.hpp>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {
    const char *backend = "8.8.8.1:53";
    const char *progname = argv[0];
    int verbose = MK_LOG_INFO;
    int chr;

    while ((chr = getopt(argc, argv, "b:v")) >= 0) {
        switch (chr) {
        case 'b':
            backend = optarg;
            break;
        case 'v':
            verbose = MK_LOG_DEBUG;
            break;
        default:
            printf("usage: %s [-v] [-b backend] input-file [...]\n", progname);
            exit(1);
        }
    }
    argc -= optind;
    argv += optind;
    if (argc <= 0) {
        printf("usage: %s [-v] [-b backend] input-file [...]\n", progname);
        exit(1);
    }

    volatile int running = 0;
    for (; argc > 0; --argc, ++argv, ++running) {
        mk::ooni::DnsInjection()
            .set_options("backend", backend)
            .set_input_filepath(argv[0])
            .set_verbosity(verbose)
            .run([&running]() { --running; });
    }

    while (running > 0) sleep(1);
}
```

We can now compile (and link) the code using the following command:

    c++ -Wall -std=c++11 -o main main.cpp -lmeasurement_kit

Then create a file named INPUT and paste inside it this content:

```
measurement-kit.github.io
nexa.polito.it
ooni.torproject.org
```

Finally, we can run the program using this input as follows:

    ./main -v INPUT INPUT INPUT

Here we repeated `INPUT` three times to show that you can schedule
three different instances of the DNS Injection test in parallel.

At this point, you may have already noticed that the backend we are
using (`8.8.8.1:53`) is not a valid DNS server. This is on purpose since
the point of the test is to spot cases where someone in the middle
maliciously injects DNS responses. To run again the test with instead
a valid DNS server (quite pointless for the purpose of the test but
still an interesting exercise), type:

    ./main -vb 8.8.8.8 INPUT
