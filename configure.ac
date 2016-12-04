# Part of measurement-kit <https://measurement-kit.github.io/>.
# Measurement-kit is free software. See AUTHORS and LICENSE for more
# information on the copying conditions.

# Autoconf requirements
AC_INIT(measurement_kit, 0.3.8, bassosimone@gmail.com)

# information on the package
AC_CONFIG_SRCDIR([Makefile.am])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE
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

# Must set -pthread before testing for -levent_pthreads
if test "`uname`" != "Darwin"; then
  CFLAGS="$CFLAGS -pthread"
  CXXFLAGS="$CXXFLAGS -pthread"
  LDFLAGS="$LDFLAGS -pthread"
fi

# On my macOS at least `/usr/local` is not in -I and -L (odd...)
if test "`uname`" = "Darwin"; then
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

MK_AM_REQUIRE_C99
MK_AM_REQUIRE_CXX11
MK_AM_REQUIRE_CXX11_LIBCXX
MK_AM_CXXFLAGS_ADD_WARNINGS

# checks for library functions

# checks for system services

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
MK_AM_PRINT_SUMMARY
