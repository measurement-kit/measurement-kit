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
AC_PROG_CXX([clang++ g++ c++])

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

    if test "$ight_libevent_path"x = "builtin"x; then
       AC_MSG_WARN([No libevent found: will use the builtin libevent])
    fi
fi

if test "$ight_libevent_path"x = "builtin"x; then
    CPPFLAGS="$CPPFLAGS -Isrc/ext/libevent/include"
    LDFLAGS="$LDFLAGS -Lsrc/ext/libevent -levent"
    AC_CONFIG_SUBDIRS([src/ext/libevent])
fi

AM_CONDITIONAL([USE_BUILTIN_LIBEVENT],
    [test "$ight_libevent_path"x = "builtin"x])

### Check for JsonCpp ###

#
# Note: Debian, Fedora and FreeBSD all install the headers
# at $prefix/include/jsoncpp/json/foo.h
#

ight_jsoncpp_path=

AC_ARG_WITH([jsoncpp],
            [AS_HELP_STRING([--with-jsoncpp],
             [C++ JSON library @<:@default=check@:>@])
            ], [
              ight_jsoncpp_path=$withval
              if test "$withval"x != "builtin"x; then
                  CPPFLAGS="$CPPFLAGS -I$withval/include"
                  LDFLAGS="$LDFLAGS -L$withval/lib"
              fi
            ], [])

if test "$ight_jsoncpp_path"x != "builtin"x; then

    AC_LANG_PUSH([C++])
    AC_CHECK_HEADERS(jsoncpp/json/json.h, [],
      [ight_jsoncpp_path=builtin])

    AC_MSG_CHECKING([for libjsoncpp])
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <jsoncpp/json/json.h>],
                                        [Json::Value root;]])],
        [
         AC_MSG_RESULT([yes])
        ]
        [],
        [
         AC_MSG_RESULT([no])
         ight_jsoncpp_path=builtin
        ]
     )
    AC_LANG_POP([C++])

    if test "$ight_jsoncpp_path"x = "builtin"x; then
       AC_MSG_WARN([No jsoncpp found: will use the builtin jsoncpp])
    else
       LDFLAGS="$LDFLAGS -ljsoncpp"
    fi
fi

if test "$ight_jsoncpp_path"x = "builtin"x; then
    CPPFLAGS="$CPPFLAGS -Isrc/ext/jsoncpp/"  # For the amalgamated source
fi

AM_CONDITIONAL([USE_BUILTIN_LIBJSONCPP],
    [test "$ight_jsoncpp_path"x = "builtin"x])

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
     AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <functional>],
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

# checks for library functions
# checks for system services

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
