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

To build the library you need a C/C++ compiler (the Makefile.in assumes
clang/clang++, and note that C++11 support is required), Python 2.7+ (in
particular, some scripts assume that there is an executable named
python), and a Unix environment. Optionally, you also need to install
[SWIG][swig] with support for Python and Java.

[swig]: http://www.swig.org/

As said, the library depends on libevent2. Under Linux the very simple
build system assumes that libevent2 is installed under /usr; on
MacOS it assumes that libevent2 is installed under /opt/local (as if
you are using MacPorts); on OpenBSD it assumes that libevent2 is
installed at /usr/local (the default place in which it is installed
if you `pkg_add -vi` it). You can edit configure, if libevent2 happens
to be installed at another place.
