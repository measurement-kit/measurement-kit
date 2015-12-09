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
AC_PROG_CXX()

AC_ARG_ENABLE([examples],
    AS_HELP_STRING([--disable-examples, skip building of examples programs]),
        [], [enable_examples=yes])
AM_CONDITIONAL([BUILD_EXAMPLES], [test "$enable_examples" = "yes"])

# checks for libraries

# LIBEVENT

measurement_kit_libevent_path=

AC_ARG_WITH([libevent],
            [AS_HELP_STRING([--with-libevent],
             [event I/O library @<:@default=check@:>@])
            ], [
              measurement_kit_libevent_path=$withval
              if test "$withval"x != "builtin"x; then
                  CPPFLAGS="$CPPFLAGS -I$withval/include"
                  LDFLAGS="$LDFLAGS -L$withval/lib"
              fi
            ], [])

if test "$measurement_kit_libevent_path"x != "builtin"x; then
    AC_CHECK_HEADERS(event2/event.h, [],
      [measurement_kit_libevent_path=builtin])

    AC_CHECK_LIB(event, event_new, [],
      [measurement_kit_libevent_path=builtin])

    AC_CHECK_HEADERS(event2/thread.h, [],
      [measurement_kit_libevent_path=builtin])

    AC_CHECK_LIB(event_pthreads, evthread_use_pthreads, [],
      [measurement_kit_libevent_path=builtin])

    if test "$measurement_kit_libevent_path"x = "builtin"x; then
       AC_MSG_WARN([No libevent found: will use the builtin libevent])
    fi
fi

if test "$measurement_kit_libevent_path"x = "builtin"x; then
    CPPFLAGS="$CPPFLAGS -I \$(top_srcdir)/src/ext/libevent/include"
    CPPFLAGS="$CPPFLAGS -I \$(top_builddir)/src/ext/libevent/include"
    LDFLAGS="$LDFLAGS -L\$(top_builddir)/src/ext/libevent -levent -levent_pthreads"
    AC_CONFIG_SUBDIRS([src/ext/libevent])
fi

AM_CONDITIONAL([USE_BUILTIN_LIBEVENT],
    [test "$measurement_kit_libevent_path"x = "builtin"x])

# JANSSON

measurement_kit_jansson_path=

AC_ARG_WITH([jansson],
            [AS_HELP_STRING([--with-jansson],
             [JSON library @<:@default=check@:>@]) ], [
              measurement_kit_jansson_path=$withval
              if test "$withval"x != "builtin"x; then
                  CPPFLAGS="$CPPFLAGS -I$withval/include"
                  LDFLAGS="$LDFLAGS -L$withval/lib"
              fi
            ], [])

if test "$measurement_kit_jansson_path"x != "builtin"x; then
    AC_CHECK_HEADERS(jansson.h, [],
      [measurement_kit_jansson_path=builtin])

    AC_CHECK_LIB(jansson, json_null, [],
      [measurement_kit_jansson_path=builtin])

    if test "$measurement_kit_jansson_path"x = "builtin"x; then
       AC_MSG_WARN([No jansson found: will use the builtin jansson])
    fi
fi

if test "$measurement_kit_jansson_path"x = "builtin"x; then
    CPPFLAGS="$CPPFLAGS -I \$(top_srcdir)/src/ext/jansson/src"
    LDFLAGS="$LDFLAGS -L\$(top_builddir)/src/ext/jansson/src -ljansson"
    AC_CONFIG_SUBDIRS([src/ext/jansson])
fi

AM_CONDITIONAL([USE_BUILTIN_JANSSON],
    [test "$measurement_kit_jansson_path"x = "builtin"x])

# LIBMAXMINDDB

measurement_kit_libmaxminddb_path=

AC_ARG_WITH([libmaxminddb],
            [AS_HELP_STRING([--with-libmaxminddb],
             [GeoIP library @<:@default=check@:>@]) ], [
              measurement_kit_libmaxminddb_path=$withval
              if test "$withval"x != "builtin"x; then
                  CPPFLAGS="$CPPFLAGS -I$withval/include"
                  LDFLAGS="$LDFLAGS -L$withval/lib"
              fi
            ], [])

if test "$measurement_kit_libmaxminddb_path"x != "builtin"x; then
    AC_CHECK_HEADERS(maxminddb.h, [],
      [measurement_kit_libmaxminddb_path=builtin])

    AC_CHECK_LIB(maxminddb, MMDB_open, [],
      [measurement_kit_libmaxminddb_path=builtin])

    if test "$measurement_kit_libmaxminddb_path"x = "builtin"x; then
       AC_MSG_WARN([No libmaxminddb found: will use the builtin libmaxminddb])
    fi
fi

if test "$measurement_kit_libmaxminddb_path"x = "builtin"x; then
    CPPFLAGS="$CPPFLAGS -I \$(top_srcdir)/src/ext/libmaxminddb/include"
    LDFLAGS="$LDFLAGS -L\$(top_builddir)/src/ext/libmaxminddb/src -lmaxminddb"
    AC_CONFIG_SUBDIRS([src/ext/libmaxminddb])
fi

AM_CONDITIONAL([USE_BUILTIN_LIBMAXMINDDB],
    [test "$measurement_kit_libmaxminddb_path"x = "builtin"x])

# YAML-CPP

# Step 1: process --with-boost and set measurement_kit_boost_path accordingly
measurement_kit_boost_path=
AC_ARG_WITH([boost],
            [AS_HELP_STRING([--with-boost],
             [Quasi-standard C++ libraries @<:@default=check@:>@])
            ], [
              measurement_kit_boost_path=$withval
            ], [])

# Step 2: if not builtin, check whether we can use $measurement_kit_boost_path
if test "$measurement_kit_boost_path"x != "builtin"x; then

    # Step 2.1: backup {CPP,LD}FLAGS and eventually augment them
    measurement_kit_saved_cppflags="$CPPFLAGS"
    if test "$measurement_kit_boost_path"x != ""x; then
        CPPFLAGS="$CPPFLAGS -I ${measurement_kit_boost_path}/include"
    fi

    AC_LANG_PUSH([C++])

    # Step 2.2: could we reach boost header neded by yaml-cpp 0.5.1?
    AC_CHECK_HEADERS(boost/iterator/iterator_adaptor.hpp, [],
      [measurement_kit_boost_path=builtin])
    AC_CHECK_HEADERS(boost/iterator/iterator_facade.hpp, [],
      [measurement_kit_boost_path=builtin])
    AC_CHECK_HEADERS(boost/noncopyable.hpp, [],
      [measurement_kit_boost_path=builtin])
    AC_CHECK_HEADERS(boost/shared_ptr.hpp, [],
      [measurement_kit_boost_path=builtin])
    AC_CHECK_HEADERS(boost/smart_ptr/shared_ptr.hpp, [],
      [measurement_kit_boost_path=builtin])
    AC_CHECK_HEADERS(boost/type_traits.hpp, [],
      [measurement_kit_boost_path=builtin])
    AC_CHECK_HEADERS(boost/utility/enable_if.hpp, [],
      [measurement_kit_boost_path=builtin])
    AC_CHECK_HEADERS(boost/utility.hpp, [],
      [measurement_kit_boost_path=builtin])

    AC_LANG_POP([C++])

    # Step 2.4: restore {CPP,LD}FLAGS if we have failed
    if test "$measurement_kit_boost_path"x = "builtin"x; then
       CPPFLAGS=$measurement_kit_saved_cppflags
    fi
fi

# Step 3: if needed, prepare to include our own boost
if test "$measurement_kit_boost_path"x = "builtin"x; then
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

# Step 1: process --with-yaml-cpp and set measurement_kit_yamlcpp_path accordingly
measurement_kit_yamlcpp_path=
AC_ARG_WITH([yaml-cpp],
            [AS_HELP_STRING([--with-yaml-cpp],
             [Library for managing YAML @<:@default=check@:>@])
            ], [
              measurement_kit_yamlcpp_path=$withval
            ], [])

# Step 2: if not builtin, check whether we can use $measurement_kit_yamlcpp_path
if test "$measurement_kit_yamlcpp_path"x != "builtin"x; then

    # Step 2.1: backup {CPP,LD}FLAGS and eventually augment them
    measurement_kit_saved_cppflags="$CPPFLAGS"
    measurement_kit_saved_ldflags="$LDFLAGS"
    if test "$measurement_kit_yamlcpp_path"x != ""x; then
        CPPFLAGS="$CPPFLAGS -I ${measurement_kit_yamlcpp_path}/include"
        LDFLAGS="$LDFLAGS -L$measurement_kit_yamlcpp_path/lib"
    fi
    LDFLAGS="$LDFLAGS -lyaml-cpp"

    AC_LANG_PUSH([C++])

    # Step 2.2: could we reach yaml-cpp header?
    AC_CHECK_HEADERS(yaml-cpp/yaml.h, [],
      [measurement_kit_yamlcpp_path=builtin])

    # Step 2.3: could we link with yaml-cpp?
    AC_MSG_CHECKING([whether we can link with yaml-cpp])
    AC_COMPILE_IFELSE([
      AC_LANG_PROGRAM(
        [[#include <yaml-cpp/yaml.h>], [YAML::Node node;]]
      )],
      [AC_MSG_RESULT([yes])]
      [],
      [
        AC_MSG_RESULT([no])
        measurement_kit_yamlcpp_path="builtin"
      ])

    AC_LANG_POP([C++])

    # Step 2.4: restore {CPP,LD}FLAGS if we have failed
    if test "$measurement_kit_yamlcpp_path"x = "builtin"x; then
       AC_MSG_WARN([No yaml-cpp found: will use the builtin yaml-cpp])
       CPPFLAGS=$measurement_kit_saved_cppflags
       LDFLAGS=$measurement_kit_saved_ldflags
    fi
fi

# Step 3: if needed, prepare to compile our own yaml-cpp
if test "$measurement_kit_yamlcpp_path"x = "builtin"x; then
    CPPFLAGS="$CPPFLAGS -I \$(top_srcdir)/src/ext/yaml-cpp/include"
    LDFLAGS="$LDFLAGS -L\$(top_builddir)/src/ext/"
fi
AM_CONDITIONAL([USE_BUILTIN_YAMLCPP],
    [test "$measurement_kit_yamlcpp_path"x = "builtin"x])

# checks for header files
# checks for types
# checks for structures

# checks for compiler characteristics

AC_PROG_CC_C99
if test x"$ac_cv_prog_cc_c99" = xno; then
    AC_MSG_ERROR([a C99 compiler is required])
fi

measurement_kit_saved_cxxflags="$CXXFLAGS"
CXXFLAGS=-std=c++11
AC_MSG_CHECKING([whether CXX supports -std=c++11])
AC_LANG_PUSH([C++])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([])],
    [AC_MSG_RESULT([yes])]
    [],
    [
     AC_MSG_RESULT([no])
     AC_MSG_ERROR([a C++11 compiler is required])
    ]
)
CXXFLAGS="$measurement_kit_saved_cxxflags -std=c++11"
AC_LANG_POP([C++])

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

AC_MSG_CHECKING([whether the C++ compiler is clang++])
if test echo | $CXX -dM -E - | grep __clang__ > /dev/null; then
    AC_MSG_RESULT([yes])
    CXXFLAGS="$CXXFLAGS -Wmissing-prototypes"
else
    AC_MSG_RESULT([yes])
fi

# checks for library functions

# checks for system services

AC_CONFIG_FILES([Makefile])

AC_OUTPUT

echo "==== configured variables ==="
echo "CC       : $CC"
echo "CXX      : $CXX"
echo "CFLAGS   : $CFLAGS"
echo "CPPFLAGS : $CPPFLAGS"
echo "CXXFLAGS : $CXXFLAGS"
echo "LDFLAGS  : $LDFLAGS"
