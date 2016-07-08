# Part of measurement-kit <https://measurement-kit.github.io/>.
# Measurement-kit is free software. See AUTHORS and LICENSE for more
# information on the copying conditions.

# Autoconf requirements
AC_INIT(measurement_kit, 0.2.2, bassosimone@gmail.com)

# information on the package
AC_CONFIG_SRCDIR([Makefile.am])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES])
LT_INIT
AC_CONFIG_HEADERS([config.h])

AM_CONDITIONAL([BUILD_TESTS], [test "yes" = "yes"])  # for now, build always

AC_CANONICAL_HOST

# checks for programs
AC_PROG_INSTALL
AC_PROG_CXX

MK_AM_ENABLE_EXAMPLES

# checks for libraries

# Must set -pthread before testing for -levent_pthreads
CFLAGS="$CFLAGS -pthread"
CXXFLAGS="$CXXFLAGS -pthread"
LDFLAGS="$LDFLAGS -pthread"

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
