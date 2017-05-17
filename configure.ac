# Part of measurement-kit <https://measurement-kit.github.io/>.
# Measurement-kit is free software. See AUTHORS and LICENSE for more
# information on the copying conditions.

# Autoconf requirements
AC_INIT(measurement_kit, 0.7.0-dev, bassosimone@gmail.com)

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

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES])
LT_INIT
AC_CONFIG_HEADERS([config.h])

AM_CONDITIONAL([BUILD_TESTS], [test "yes" = "yes"])  # for now, build always

AC_CANONICAL_HOST
AC_PROG_INSTALL

# Must be before AC_PROG_CXX (see http://stackoverflow.com/questions/11703709)
MK_AM_ENABLE_COVERAGE
MK_AM_ADD_COVERAGE_FLAGS_IF_NEEDED
MK_AM_DISABLE_BINARIES
MK_AM_DISABLE_EXAMPLES
MK_AM_DISABLE_INTEGRATION_TESTS
MK_AM_DISABLE_TRACEROUTE

# See above comment
AC_PROG_CXX

if test "`uname`" != "Darwin"; then
  # Must set -pthread before testing for -levent_pthreads
  CFLAGS="$CFLAGS -pthread"
  CXXFLAGS="$CXXFLAGS -pthread"
  LDFLAGS="$LDFLAGS -pthread"
elif test "$cross_compiling" != yes; then
  # On my macOS, /usr/local is not added automatically
  CPPFLAGS="$CPPFLAGS -I/usr/local/include"
  LDFLAGS="$LDFLAGS -L/usr/local/lib"
fi

MK_AM_CHECK_LIBC_FUNCS
MK_AM_OPENSSL
MK_AM_LIBEVENT
MK_AM_GEOIP

# checks for header files
# checks for types
# checks for structures

# checks for ca-bundle
MK_CHECK_CA_BUNDLE

# checks for compiler characteristics

CPPFLAGS="-I \$(top_srcdir)/include $CPPFLAGS"
CFLAGS="-Wall -Wextra -pedantic $CFLAGS"
CXXFLAGS="-Wall -Wextra -pedantic $CXXFLAGS"

# Force compiling the http_parser in non-strict mode. This allows us to test
# partially broken sites like mail.voila.fr that send spaces between the header
# key and the separator (i.e. ':').
CPPFLAGS="-DHTTP_PARSER_STRICT=0 $CPPFLAGS"

MK_AM_REQUIRE_C99
MK_AM_REQUIRE_CXX11
MK_AM_REQUIRE_CXX11_LIBCXX
MK_AM_CXXFLAGS_ADD_WARNINGS

# checks for library functions

# checks for system services

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
MK_AM_PRINT_SUMMARY
