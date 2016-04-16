dnl Part of measurement-kit <https://measurement-kit.github.io/>.
dnl Measurement-kit is free software. See AUTHORS and LICENSE for more
dnl information on the copying conditions.


AC_DEFUN([MK_AM_ENABLE_EXAMPLES], [
AC_ARG_ENABLE([examples],
    AS_HELP_STRING([--disable-examples, skip building of examples programs]),
        [], [enable_examples=yes])
AM_CONDITIONAL([BUILD_EXAMPLES], [test "$enable_examples" = "yes"])
])


AC_DEFUN([MK_AM_LIBEVENT], [
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
])


AC_DEFUN([MK_AM_JANSSON], [
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
])


AC_DEFUN([MK_AM_LIBMAXMINDDB], [
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
])


AC_DEFUN([MK_AM_REQUIRE_C99], [
AC_PROG_CC_C99
if test x"$ac_cv_prog_cc_c99" = xno; then
    AC_MSG_ERROR([a C99 compiler is required])
fi
])


AC_DEFUN([MK_AM_REQUIRE_CXX11], [
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
])


AC_DEFUN([MK_AM_REQUIRE_CXX11_LIBCXX], [
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
])


AC_DEFUN([MK_AM_CXXFLAGS_ADD_WARNINGS], [
AC_MSG_CHECKING([whether the C++ compiler is clang++])
if test echo | $CXX -dM -E - | grep __clang__ > /dev/null; then
    AC_MSG_RESULT([yes])
    CXXFLAGS="$CXXFLAGS -Wmissing-prototypes"
else
    AC_MSG_RESULT([yes])
fi
])


AC_DEFUN([MK_AM_PRINT_SUMMARY], [
echo "==== configured variables ==="
echo "CC       : $CC"
echo "CXX      : $CXX"
echo "CFLAGS   : $CFLAGS"
echo "CPPFLAGS : $CPPFLAGS"
echo "CXXFLAGS : $CXXFLAGS"
echo "LDFLAGS  : $LDFLAGS"
])
