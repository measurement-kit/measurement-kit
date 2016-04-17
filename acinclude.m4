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
  echo "> checking for dependency: libevent"

  AC_ARG_WITH([libevent],
              [AS_HELP_STRING([--with-libevent],
                [event I/O library @<:@default=check@:>@])
              ],
              [
                CPPFLAGS="$CPPFLAGS -I$withval/include"
                LDFLAGS="$LDFLAGS -L$withval/lib"
              ],
              [])

  mk_not_found=""
  AC_CHECK_HEADERS(event2/event.h, [], [mk_not_found=1])
  AC_CHECK_LIB(event, event_new, [], [mk_not_found=1])
  AC_CHECK_HEADERS(event2/thread.h, [], [mk_not_found=1])
  AC_CHECK_LIB(event_pthreads, evthread_use_pthreads, [], [mk_not_found=1])

  if test "$mk_not_found" = "1"; then
    AC_MSG_WARN([Failed to find dependency: libevent])
    echo "    - to install on Debian: sudo apt-get install libevent-dev"
    echo "    - to install on OSX: brew install libevent"
    echo "    - to compile from sources: ./build/dependency libevent"
    AC_MSG_ERROR([Please, install libevent and run configure again])
  fi
  echo ""
])


AC_DEFUN([MK_AM_JANSSON], [
  echo "> checking for dependency: jansson"

  AC_ARG_WITH([jansson],
              [AS_HELP_STRING([--with-jansson],
                [JSON library @<:@default=check@:>@])
              ],
              [
                CPPFLAGS="$CPPFLAGS -I$withval/include"
                LDFLAGS="$LDFLAGS -L$withval/lib"
              ],
              [])

  mk_not_found=""
  AC_CHECK_HEADERS(jansson.h, [], [mk_not_found=1])
  AC_CHECK_LIB(jansson, json_null, [], [mk_not_found=1])

  if test "$mk_not_found" = "1"; then
    AC_MSG_WARN([Failed to find dependency: jansson])
    echo "    - to install on Debian: sudo apt-get install libjansson-dev"
    echo "    - to install on OSX: brew install jansson"
    echo "    - to compile from sources: ./build/dependency jansson"
    AC_MSG_ERROR([Please, install jansson and run configure again])
  fi
  echo ""
])


AC_DEFUN([MK_AM_LIBMAXMINDDB], [
  echo "> checking for dependency: libmaxminddb"

  AC_ARG_WITH([libmaxminddb],
              [AS_HELP_STRING([--with-libmaxminddb],
                [GeoIP library @<:@default=check@:>@])
              ],
              [
                CPPFLAGS="$CPPFLAGS -I$withval/include"
                LDFLAGS="$LDFLAGS -L$withval/lib"
              ],
              [])

  mk_not_found=""
  AC_CHECK_HEADERS(maxminddb.h, [], [mk_not_found=1])
  AC_CHECK_LIB(maxminddb, MMDB_open, [], [mk_not_found=1])

  if test "$mk_not_found" = "1"; then
    AC_MSG_WARN([Failed to find dependency: libmaxminddb])
    echo "    - to install on Debian: sudo apt-get install libmaxminddb-dev"
    echo "    - to install on OSX: brew install libmaxminddb"
    echo "    - to compile from sources: ./build/dependency libmaxminddb"
    AC_MSG_ERROR([Please, install libmaxminddb and run configure again])
  fi
  echo ""
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
