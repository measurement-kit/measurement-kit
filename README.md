LibNeubot
=========

LibNeubot is an experimental support library for [Neubot][neubot] (especially
for [Neubot for Android][neubot_android]) that is written in C/C++ and based
on [libevent2].

[neubot_android]: https://github.com/neubot/neubot_android
[neubot]: https://github.com/neubot/neubot
[libevent2]: https://github.com/libevent/libevent

I have successfully compiled and run its simple tests in the following
systems: MacOS 10.8, OpenBSD 5.5-current, Ubuntu 13.10.

To build the library you need a C/C++ compiler, Python 2.7+ (in
particular, `./config.local` assume that there is an executable named
python), and a Unix environment.

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
