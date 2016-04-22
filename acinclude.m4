dnl Part of measurement-kit <https://measurement-kit.github.io/>.
dnl Measurement-kit is free software. See AUTHORS and LICENSE for more
dnl information on the copying conditions.

AC_DEFUN([MK_AM_ENABLE_COVERAGE], [
  AC_ARG_ENABLE([coverage],
    AS_HELP_STRING([--enable-coverage, build for coverage]),
      [enable_coverage=yes], [])
])

AC_DEFUN([MK_AM_ADD_COVERAGE_FLAGS_IF_NEEDED], [
  if test "$enable_coverage" = "yes"; then
    CFLAGS="--coverage $CFLAGS"
    CXXFLAGS="--coverage $CXXFLAGS"
    LDFLAGS="--coverage $LDFLAGS"
  fi
])

AC_DEFUN([MK_AM_DISABLE_EXAMPLES], [
  AC_ARG_ENABLE([examples],
    AS_HELP_STRING([--disable-examples, skip building of examples programs]),
                   [], [enable_examples=yes])
  AM_CONDITIONAL([BUILD_EXAMPLES], [test "$enable_examples" = "yes"])
])

dnl TODO: this should be probably become --disable-network-tests
AC_DEFUN([MK_AM_DISABLE_TESTS], [
  AC_ARG_ENABLE([tests],
    AS_HELP_STRING([--disable-tests, skip building and running test programs]),
      [], [enable_tests=yes])
  AM_CONDITIONAL([BUILD_TESTS], [test "$enable_tests" = "yes"])
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
  AC_CHECK_LIB(event_openssl, bufferevent_openssl_filter_new, [],
               [mk_not_found=1])

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

AC_DEFUN([MK_AM_OPENSSL], [
  echo "> checking for dependency: openssl"

  AC_ARG_WITH([openssl],
              [AS_HELP_STRING([--with-openssl],
                [SSL toolkit @<:@default=check@:>@])
              ],
              [
                CPPFLAGS="$CPPFLAGS -I$withval/include"
                LDFLAGS="$LDFLAGS -L$withval/lib"
              ],
              [])

  mk_not_found=""
  AC_CHECK_HEADERS(openssl/ssl.h, [], [mk_not_found=1])
  AC_CHECK_LIB(crypto, RSA_new, [], [mk_not_found=1])
  AC_CHECK_LIB(ssl, SSL_new, [], [mk_not_found=1])

  AC_MSG_CHECKING([whether OpenSSL is older than 1.0.0])
  AC_RUN_IFELSE([
    AC_LANG_SOURCE([
      #include <openssl/opensslv.h>
      #include <openssl/ssl.h>

      /* Code taken from tor/src/common/crypto.h */

      #define OPENSSL_VER(a,b,c,d,e)                                \
        (((a)<<28) |                                                \
         ((b)<<20) |                                                \
         ((c)<<12) |                                                \
         ((d)<< 4) |                                                \
          (e))
      #define OPENSSL_V_SERIES(a,b,c) OPENSSL_VER((a),(b),(c),0,0)

      /* Code taken from tor/src/common/compat_openssl.h */

      #if OPENSSL_VERSION_NUMBER < OPENSSL_V_SERIES(1,0,0)
      # error "We require OpenSSL >= 1.0.0"
      #endif

      #if OPENSSL_VERSION_NUMBER >= OPENSSL_V_SERIES(1,1,0) && \
          ! defined(LIBRESSL_VERSION_NUMBER)
      /* We define this macro if we're trying to build with the majorly refactored
       * API in OpenSSL 1.1 */
      #define OPENSSL_1_1_API
      #endif

      #ifndef OPENSSL_1_1_API
      #define OpenSSL_version_num() SSLeay()
      #endif

      int main() {
        if (OpenSSL_version_num() != OPENSSL_VERSION_NUMBER) {
          return 1;
        }
        if (OpenSSL_version_num() < OPENSSL_V_SERIES(1, 0, 0)) {
          return 2;
        }
        return 0;
      }
    ])
  ],
  [AC_MSG_RESULT([yes])],
  [
    AC_MSG_RESULT([no])
    if test /usr/local/Cellar/openssl; then
      AC_MSG_WARN([MacOS ships an old system-wide OpenSSL but you seem to])
      AC_MSG_WARN([have a new version installed with brew.])
      AC_MSG_WARN([So, you should try adding this flag to configure:])
      AC_MSG_WARN(['--with-openssl=/usr/local/Cellar/openssl/VERSION/'])
    fi
    mk_not_found=1
  ])

  if test "$mk_not_found" = "1"; then
    AC_MSG_WARN([Failed to find dependency: openssl])
    echo "    - to install on Debian: sudo apt-get install libssl-dev"
    echo "    - to install on OSX: brew install openssl"
    echo "    - to compile from sources: ./build/dependency libressl"
    AC_MSG_ERROR([Please, install openssl and run configure again])
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
  mk_saved_cxxflags="$CXXFLAGS"
  CXXFLAGS=-std=c++11
  AC_MSG_CHECKING([whether CXX supports -std=c++11])
  AC_LANG_PUSH([C++])
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([])],
    [AC_MSG_RESULT([yes])]
    [],
    [
     AC_MSG_RESULT([no])
     AC_MSG_ERROR([a C++11 compiler is required])
    ])
  CXXFLAGS="$mk_saved_cxxflags -std=c++11"
  AC_LANG_POP([C++])
])

dnl This should probably be removed since MacOS 10.8 is now obsolete
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
  AC_MSG_CHECKING([whether compiler is clang to add clang specific warnings])
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
