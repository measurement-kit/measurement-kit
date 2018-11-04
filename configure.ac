# Autoconf requirements
AC_INIT(measurement_kit, 0.9.0-alpha.10, bassosimone@gmail.com)

# information on the package
AC_CONFIG_SRCDIR([Makefile.am])
AC_CONFIG_MACRO_DIR([m4])

# Travis uses Ubuntu 12.04 where the default automake was 1.11 in which
# by default tests were run in serial mode; as such, the LOG_COMPILER hack
# used to run tests using Valgrind was not working because automake 1.11
# wasn't honouring the LOG_COMPILER definition in the Makefile.
#
# Force parallel tests to make sure we can run tests through Valgrind.
AM_INIT_AUTOMAKE([parallel-tests 1.11])

LT_INIT([disable-shared], [pic-only])
AC_CONFIG_HEADERS([config.h])

AC_CANONICAL_HOST
AC_PROG_INSTALL

case "$host" in
  *-apple-darwin*)
    if test "$cross_compiling" != yes; then
      # On my macOS, /usr/local is not added automatically
      CPPFLAGS="$CPPFLAGS -I/usr/local/include"
      LDFLAGS="$LDFLAGS -L/usr/local/lib"
    fi
  ;;
  *-w64-mingw32)
    # Must link with ws2_32
    LIBS="$LIBS -lws2_32"
    # Required to expose inet_pton()
    CPPFLAGS="$CPPFLAGS -D_WIN32_WINNT=0x0600"
  ;;
esac

AC_PROG_CXX

MK_MAYBE_ADD_CFLAG([-pthread])
MK_MAYBE_ADD_CXXFLAG([-pthread])
MK_MAYBE_ADD_LDFLAG([-pthread])

MK_AM_CHECK_LIBC_FUNCS
MK_AM_OPENSSL
MK_AM_LIBEVENT
MK_AM_GEOIP
MK_AM_RESOLV
MK_AM_LIBCURL
MK_AM_LIBMAXMINDDB

# checks for header files
# checks for types
# checks for structures

# checks for ca-bundle
MK_CHECK_CA_BUNDLE

# checks for compiler characteristics

CPPFLAGS="-I \$(top_srcdir)/include $CPPFLAGS"

for flag in -Wall -Wextra -pedantic -Wmissing-prototypes; do
  MK_MAYBE_ADD_CFLAG([$flag])
  MK_MAYBE_ADD_CXXFLAG([$flag])
done

# Force compiling the http_parser in non-strict mode. This allows us to test
# partially broken sites like mail.voila.fr that send spaces between the header
# key and the separator (i.e. ':').
CPPFLAGS="-DHTTP_PARSER_STRICT=0 $CPPFLAGS"

# Make sure the build is not gonna fail for deprecated stuff.
CPPFLAGS="-DMK_NETTESTS_INTERNAL $CPPFLAGS"

MK_REQUIRE_CFLAG([-std=c11])
MK_REQUIRE_CXXFLAG([-std=c++14])

# checks for library functions

# checks for system services

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
