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

MKIT_FIND_LIBEVENT
if test "$MKIT_LIBEVENT" = "no"; then
    AC_MSG_WARN([No libevent found: will use the builtin libevent])
    CPPFLAGS="$CPPFLAGS -I \$(top_srcdir)/src/ext/libevent/include"
    CPPFLAGS="$CPPFLAGS -I \$(top_builddir)/src/ext/libevent/include"
    LDFLAGS="$LDFLAGS -L\$(top_builddir)/src/ext/libevent -levent -levent_pthreads"
    AC_CONFIG_SUBDIRS([src/ext/libevent])
fi
AM_CONDITIONAL([USE_BUILTIN_LIBEVENT], [test "$MKIT_LIBEVENT" = "no"])

MKIT_FIND_JANSSON
if test "$MKIT_JANSSON" = "no"; then
    AC_MSG_WARN([No jansson found: will use the builtin jansson])
    CPPFLAGS="$CPPFLAGS -I \$(top_srcdir)/src/ext/jansson/src"
    LDFLAGS="$LDFLAGS -L\$(top_builddir)/src/ext/jansson/src -ljansson"
    AC_CONFIG_SUBDIRS([src/ext/jansson])
fi
AM_CONDITIONAL([USE_BUILTIN_JANSSON], [test "$MKIT_JANSSON" = "no"])

MKIT_FIND_LIBMAXMINDDB
if test "$MKIT_LIBMAXMINDDB" = "no"; then
    AC_MSG_WARN([No libmaxminddb found: will use the builtin libmaxminddb])
    CPPFLAGS="$CPPFLAGS -I \$(top_srcdir)/src/ext/libmaxminddb/include"
    LDFLAGS="$LDFLAGS -L\$(top_builddir)/src/ext/libmaxminddb/src -lmaxminddb"
    AC_CONFIG_SUBDIRS([src/ext/libmaxminddb])
fi
AM_CONDITIONAL([USE_BUILTIN_LIBMAXMINDDB], [test "$MKIT_LIBMAXMINDDB" = "no"])

MKIT_FIND_BOOST
if test "$MKIT_BOOST" = "no"; then
    AC_MSG_WARN([No boost found: will use the builtin boost])
    CPPFLAGS="$CPPFLAGS -I \$(top_srcdir)/src/ext/boost/assert/include"
    CPPFLAGS="$CPPFLAGS -I \$(top_srcdir)/src/ext/boost/config/include"
    CPPFLAGS="$CPPFLAGS -I \$(top_srcdir)/src/ext/boost/core/include"
    CPPFLAGS="$CPPFLAGS -I \$(top_srcdir)/src/ext/boost/detail/include"
    CPPFLAGS="$CPPFLAGS -I \$(top_srcdir)/src/ext/boost/iterator/include"
    CPPFLAGS="$CPPFLAGS -I \$(top_srcdir)/src/ext/boost/mpl/include"
    CPPFLAGS="$CPPFLAGS -I \$(top_srcdir)/src/ext/boost/predef/include"
    CPPFLAGS="$CPPFLAGS -I \$(top_srcdir)/src/ext/boost/preprocessor/include"
    CPPFLAGS="$CPPFLAGS -I \$(top_srcdir)/src/ext/boost/smart_ptr/include"
    CPPFLAGS="$CPPFLAGS -I \$(top_srcdir)/src/ext/boost/static_assert/include"
    CPPFLAGS="$CPPFLAGS -I \$(top_srcdir)/src/ext/boost/throw_exception/include"
    CPPFLAGS="$CPPFLAGS -I \$(top_srcdir)/src/ext/boost/type_traits/include"
    CPPFLAGS="$CPPFLAGS -I \$(top_srcdir)/src/ext/boost/typeof/include"
    CPPFLAGS="$CPPFLAGS -I \$(top_srcdir)/src/ext/boost/utility/include"
fi

MKIT_FIND_YAML_CPP
if test "$MKIT_YAML_CPP" = "no"; then
    AC_MSG_WARN([No yaml-cpp found: will use the builtin yaml-cpp])
    CPPFLAGS="$CPPFLAGS -I \$(top_srcdir)/src/ext/yaml-cpp/include"
    LDFLAGS="$LDFLAGS -L\$(top_builddir)/src/ext/"
fi
AM_CONDITIONAL([USE_BUILTIN_YAMLCPP], [test "$MKIT_YAML_CPP" = "no"])

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
