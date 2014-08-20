Libight
=======

Libight is an experimental library that provides common functionalities
useful to implement open measurement tools on mobile platforms.

As of version 3.0.0, we have successfully compiled and run its tests in the
following systems: MacOS 10.8, OpenBSD 5.5-current, Ubuntu 13.10.

To build the library you need a C/C++ compiler and a Unix environment. (The
`./configure` script checks whether all the dependencies are in place.)

Before running any of the below build commands be sure to initialize the
submodules with:
    
    git submodule update --init

How to build it
---------------

    autoreconf -i
    ./configure
    make
    make check

If libevent is not found, the build system will configure and
compile the built-in libevent (`src/ext/libevent`). To force
`./configure` to use the libevent at `DIR`, consider using the
`--with-libevent=DIR` option. To force `./configure` to use
the bult-in libevent, specify `--with-libevent=builtin`. For
example, if you want to force `./configure` to use the libevent
installed at `/opt/local`, run:

    ./configure --with-libevent=/opt/local

To tell make to produce less output (as in the Linux kernel
build process) run:

    make V=0
