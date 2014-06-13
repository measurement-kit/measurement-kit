Libight
=======

Libight is an experimental library that provides common functionalities
useful to implement open measurement tools on mobile platforms.

As of version 3.0.0, we have successfully compiled and run its tests in the
following systems: MacOS 10.8, OpenBSD 5.5-current, Ubuntu 13.10.

To build the library you need a C/C++ compiler and a Unix environment. (The
`./configure` script checks whether all the dependencies are in place.)

How to build it
---------------

    autoreconf -i
    ./configure
    make
    make check

If `./configure` does not find libevent, consider using the
`--with-libevent=DIR` option. For example, if libevent is
installed below `/opt/local`, run:

    ./configure --with-libevent=/opt/local

To tell make to produce less output (as in the Linux kernel
build process) run:

    make V=0
