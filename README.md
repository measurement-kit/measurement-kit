LibNeubot
=========

LibNeubot is an experimental support library for [Neubot][neubot] that
is written in C and based on [libevent].

[neubot]: https://github.com/neubot/neubot
[libevent]: https://github.com/libevent/libevent

I have successfully compiled and run its simple tests in the following
systems: MacOS 10.8, OpenBSD 5.4-current, Ubuntu 13.10.

To build the library you need a C compiler (the Makefile.in assumes
clang, but you can edit it), Python (in particular, some scripts assume
that there is an executable named python), and a Unix environment.

As said, the library depends on libevent. Under Linux the very simple
build system assumes that libevent is installed under /usr, while on
MacOS it assumes that libevent is installed under /opt/local. You can
edit configure, if libevent is installed in another place.

I have tested libneubot both against libevent 1.4 and against
libevent 2.0.
