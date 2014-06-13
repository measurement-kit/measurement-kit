# configure.ac

#
# This file is part of Libight <https://libight.github.io/>.
#
# Libight is free software. See AUTHORS and LICENSE for more
# information on the copying conditions.
#

# Autoconf requirements
AC_INIT(libight, 4.0.0, bassosimone@gmail.com)

# information on the package
AC_CONFIG_SRCDIR([src/ight_wrappers.cpp])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES])
LT_INIT
AC_CONFIG_HEADERS([config.h])

AC_CANONICAL_HOST

# checks for programs
AC_PROG_INSTALL
AC_PROG_CXX([clang++ g++ c++])
AM_PATH_PYTHON([2.6])

# checks for libraries

AC_ARG_WITH([libevent],
            [AS_HELP_STRING([--with-libevent],
             [event I/O library @<:@default=check@:>@])
            ], [
              CPPFLAGS="$CPPFLAGS -I$withval/include"
              LDFLAGS="$LDFLAGS -L$withval/lib"
            ], [])
AC_CHECK_HEADERS(event2/event.h, [],
  AC_MSG_ERROR([unable to find libevent headers]))
AC_CHECK_LIB(event, event_new, [],
  AC_MSG_ERROR([unable to find libevent library]))

# checks for header files
# checks for types
# checks for structures

# checks for compiler characteristics

AC_PROG_CC_C99
if test x"$ac_cv_prog_cc_c99" = xno; then
    AC_MSG_ERROR([a C99 compiler is required])
fi

ight_saved_cxxflags="$CXXFLAGS"
CXXFLAGS=-std=c++11
AC_MSG_CHECKING([whether CXX supports -std=c++11])
AC_LANG_PUSH([C++])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([])],
    [AC_MSG_RESULT([yes])]
    [],
    [AC_MSG_ERROR([a C++11 compiler is required])]
)
CXXFLAGS="$ight_saved_cxxflags -std=c++11"
AC_LANG_POP([C++])

# checks for library functions
# checks for system services

AC_CONFIG_FILES([Makefile])

AC_OUTPUT

./config.local
