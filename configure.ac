# configure.ac

# Part of measurement-kit <https://measurement-kit.github.io/>.
# Measurement-kit is free software. See AUTHORS and LICENSE for more
# information on the copying conditions.

# Autoconf requirements
AC_INIT(measurement_kit, 0.1.0, bassosimone@gmail.com)

# information on the package
AC_CONFIG_SRCDIR([src/common/poller.cpp])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES])
LT_INIT
AC_CONFIG_HEADERS([config.h])

AC_CANONICAL_HOST

# checks for programs
AC_PROG_INSTALL
AC_PROG_CXX

MKIT_ARG_DISABLE_EXAMPLES

# checks for libraries and for header files

# Must set -pthread before testing for -levent_pthreads
CFLAGS="$CFLAGS -pthread"
CXXFLAGS="$CXXFLAGS -pthread"
LDFLAGS="$LDFLAGS -pthread"

MKIT_AM_CATCH
MKIT_AM_LIBEVENT
MKIT_AM_HTTP_PARSER
MKIT_AM_JANSSON
MKIT_AM_LIBMAXMINDDB
MKIT_AM_BOOST
MKIT_AM_YAML_CPP

MKIT_REQUIRE_DEPENDENCIES

# checks for types
# checks for structures

# checks for compiler characteristics

AC_PROG_CC_C99
if test x"$ac_cv_prog_cc_c99" = xno; then
    AC_MSG_ERROR([a C99 compiler is required])
fi

MKIT_REQUIRE_CXX11

measurement_kit_saved_cxxflags="$CXXFLAGS"
CXXFLAGS="-std=c++11"
measurement_kit_cxx_stdlib_flags=""
AC_MSG_CHECKING([whether the C++ library supports C++11])
AC_LANG_PUSH([C++])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <functional>],
                                    [std::function<void(void)> f;]])],
    [AC_MSG_RESULT([yes])]
    [],
    [
     AC_MSG_RESULT([no])
     #
     # Special case for MacOS 10.8, in which we need to explicitly
     # tell the compiler to use libc++ (which supports C++11).
     #
     AC_MSG_CHECKING([whether libc++ is available])
     CXXFLAGS="-std=c++11 -stdlib=libc++"
     AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <functional>]
                                         [std::function<void(void)> f;]])],
        [
         AC_MSG_RESULT([yes])
         measurement_kit_cxx_stdlib_flags="-stdlib=libc++"
        ]
        [],
        [
         AC_MSG_RESULT([no])
         AC_MSG_ERROR([a C++11 library is required])
        ]
     )
    ]
)
CXXFLAGS="$measurement_kit_saved_cxxflags $measurement_kit_cxx_stdlib_flags"
AC_LANG_POP([C++])

MKIT_ADD_WARNINGS_TO_CXXFLAGS

# checks for library functions

# checks for system services

AC_CONFIG_FILES([Makefile])

AC_OUTPUT

MKIT_PRINT_SUMMARY
