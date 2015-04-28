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

# checks for libraries

ight_libevent_path=

AC_ARG_WITH([libevent],
            [AS_HELP_STRING([--with-libevent],
             [event I/O library @<:@default=check@:>@])
            ], [
              ight_libevent_path=$withval
              if test "$withval"x != "builtin"x; then
                  CPPFLAGS="$CPPFLAGS -I$withval/include"
                  LDFLAGS="$LDFLAGS -L$withval/lib"
              fi
            ], [])

if test "$ight_libevent_path"x != "builtin"x; then
    AC_CHECK_HEADERS(event2/event.h, [],
      [ight_libevent_path=builtin])

    AC_CHECK_LIB(event, event_new, [],
      [ight_libevent_path=builtin])

    AC_CHECK_HEADERS(event2/thread.h, [],
      [ight_libevent_path=builtin])

    AC_CHECK_LIB(event_pthreads, evthread_use_pthreads, [],
      [ight_libevent_path=builtin])

    if test "$ight_libevent_path"x = "builtin"x; then
       AC_MSG_WARN([No libevent found: will use the builtin libevent])
    fi
fi

if test "$ight_libevent_path"x = "builtin"x; then
    CPPFLAGS="$CPPFLAGS -I \$(top_srcdir)/src/ext/libevent/include"
    CPPFLAGS="$CPPFLAGS -I \$(top_builddir)/src/ext/libevent/include"
    LDFLAGS="$LDFLAGS -L\$(top_builddir)/src/ext/libevent -levent -levent_pthreads"
    AC_CONFIG_SUBDIRS([src/ext/libevent])
fi

AM_CONDITIONAL([USE_BUILTIN_LIBEVENT],
    [test "$ight_libevent_path"x = "builtin"x])



# Step 1: process --with-libboost and set ight_boost_path accordingly
ight_boost_path=
AC_ARG_WITH([libboost],
            [AS_HELP_STRING([--with-libboost],
             [Quasi-standard C++ libraries @<:@default=check@:>@])
            ], [
              ight_boost_path=$withval
            ], [])

# Step 2: if not builtin, check whether we can use $ight_boost_path
if test "$ight_boost_path"x != "builtin"x; then

    # Step 2.1: backup {CPP,LD}FLAGS and eventually augment them
    ight_saved_cppflags="$CPPFLAGS"
    if test "$ight_boost_path"x != ""x; then
        CPPFLAGS="$CPPFLAGS -I ${ight_boost_path}/include"
    fi

    AC_LANG_PUSH([C++])

    # Step 2.2: could we reach boost header neded by yaml-cpp 0.5.1?
    AC_CHECK_HEADERS(boost/iterator/iterator_adaptor.hpp, [],
      [ight_boost_path=builtin])
    AC_CHECK_HEADERS(boost/iterator/iterator_facade.hpp, [],
      [ight_boost_path=builtin])
    AC_CHECK_HEADERS(boost/noncopyable.hpp, [],
      [ight_boost_path=builtin])
    AC_CHECK_HEADERS(boost/shared_ptr.hpp, [],
      [ight_boost_path=builtin])
    AC_CHECK_HEADERS(boost/smart_ptr/shared_ptr.hpp, [],
      [ight_boost_path=builtin])
    AC_CHECK_HEADERS(boost/type_traits.hpp, [],
      [ight_boost_path=builtin])
    AC_CHECK_HEADERS(boost/utility/enable_if.hpp, [],
      [ight_boost_path=builtin])
    AC_CHECK_HEADERS(boost/utility.hpp, [],
      [ight_boost_path=builtin])

    AC_LANG_POP([C++])

    # Step 2.4: restore {CPP,LD}FLAGS if we have failed
    if test "$ight_boost_path"x = "builtin"x; then
       CPPFLAGS=$ight_saved_cppflags
    fi
fi

# Step 3: if needed, prepare to include our own libboost
if test "$ight_boost_path"x = "builtin"x; then
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



# Step 1: process --with-libyaml-cpp and set ight_yamlcpp_path accordingly
ight_yamlcpp_path=
AC_ARG_WITH([libyaml-cpp],
            [AS_HELP_STRING([--with-libyaml-cpp],
             [Library for managing YAML @<:@default=check@:>@])
            ], [
              ight_yamlcpp_path=$withval
            ], [])

# Step 2: if not builtin, check whether we can use $ight_yamlcpp_path
if test "$ight_yamlcpp_path"x != "builtin"x; then

    # Step 2.1: backup {CPP,LD}FLAGS and eventually augment them
    ight_saved_cppflags="$CPPFLAGS"
    ight_saved_ldflags="$LDFLAGS"
    if test "$ight_yamlcpp_path"x != ""x; then
        CPPFLAGS="$CPPFLAGS -I ${ight_yamlcpp_path}/include"
        LDFLAGS="$LDFLAGS -L$ight_yamlcpp_path/lib"
    fi
    LDFLAGS="$LDFLAGS -lyaml-cpp"

    AC_LANG_PUSH([C++])

    # Step 2.2: could we reach yaml-cpp header?
    AC_CHECK_HEADERS(yaml-cpp/yaml.h, [],
      [ight_yamlcpp_path=builtin])

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
        ight_yamlcpp_path="builtin"
      ])

    AC_LANG_POP([C++])

    # Step 2.4: restore {CPP,LD}FLAGS if we have failed
    if test "$ight_yamlcpp_path"x = "builtin"x; then
       AC_MSG_WARN([No yaml-cpp found: will use the builtin yaml-cpp])
       CPPFLAGS=$ight_saved_cppflags
       LDFLAGS=$ight_saved_ldflags
    fi
fi

# Step 3: if needed, prepare to compile our own yaml-cpp
if test "$ight_yamlcpp_path"x = "builtin"x; then
    CPPFLAGS="$CPPFLAGS -I \$(top_srcdir)/src/ext/yaml-cpp/include"
    LDFLAGS="$LDFLAGS -L\$(top_builddir)/src/ext/"
fi
AM_CONDITIONAL([USE_BUILTIN_YAMLCPP],
    [test "$ight_yamlcpp_path"x = "builtin"x])



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
    [
     AC_MSG_RESULT([no])
     AC_MSG_ERROR([a C++11 compiler is required])
    ]
)
CXXFLAGS="$ight_saved_cxxflags -std=c++11"
AC_LANG_POP([C++])

ight_saved_cxxflags="$CXXFLAGS"
CXXFLAGS="-std=c++11"
ight_cxx_stdlib_flags=""
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
         ight_cxx_stdlib_flags="-stdlib=libc++"
        ]
        [],
        [
         AC_MSG_RESULT([no])
         AC_MSG_ERROR([a C++11 library is required])
        ]
     )
    ]
)
CXXFLAGS="$ight_saved_cxxflags $ight_cxx_stdlib_flags"
AC_LANG_POP([C++])

ight_saved_cxxflags="$CXXFLAGS"
CXXFLAGS="-Werror -Wmissing-prototypes"
ight_cxx_stdlib_flags=""
AC_MSG_CHECKING([whether the C++ compiler supports -Wmissing-prototypes])
AC_LANG_PUSH([C++])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <string>]
                                    [std::string s;]])],
    [
     AC_MSG_RESULT([yes])
     ight_cxx_missing_prototypes="-Wmissing-prototypes"
    ]
    [],
    [
     AC_MSG_RESULT([no])
    ]
)
CXXFLAGS="$ight_saved_cxxflags $ight_cxx_missing_prototypes"
AC_LANG_POP([C++])

# checks for library functions
# checks for system services

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
