# configure.ac

# Part of measurement-kit <https://measurement-kit.github.io/>.
# Measurement-kit is free software. See AUTHORS and LICENSE for more
# information on the copying conditions.

# Autoconf requirements
AC_INIT(measurement_kit, 0.2.0-alpha, bassosimone@gmail.com)

# information on the package
AC_CONFIG_SRCDIR([Makefile.am])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES])
LT_INIT
AC_CONFIG_HEADERS([config.h])

AC_CANONICAL_HOST

# checks for programs
AC_PROG_INSTALL
AC_PROG_CXX()

MK_AM_ENABLE_EXAMPLES

# checks for libraries

MK_AM_LIBEVENT
MK_AM_JANSSON
MK_AM_LIBMAXMINDDB
MK_AM_BOOST
MK_AM_YAML_CPP

# checks for header files
# checks for types
# checks for structures

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
