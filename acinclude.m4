AC_DEFUN([MK_AM_ENABLE_COVERAGE], [
  AC_ARG_ENABLE([coverage],
    AS_HELP_STRING([--enable-coverage, build for coverage]),
      [enable_coverage=yes], [])
])

AC_DEFUN([MK_AM_ADD_COVERAGE_FLAGS_IF_NEEDED], [
  if test "$enable_coverage" = "yes"; then
    CFLAGS="$CFLAGS --coverage -g -O0"
    CXXFLAGS="$CXXFLAGS --coverage -g -O0"
    LDFLAGS="$LDFLAGS --coverage"
  fi
])

AC_DEFUN([MK_AM_DISABLE_EXAMPLES], [
  AC_ARG_ENABLE([examples],
    AS_HELP_STRING([--disable-examples, skip building of examples programs]),
                   [], [enable_examples=yes])
  AM_CONDITIONAL([BUILD_EXAMPLES], [test "$enable_examples" = "yes"])
])

AC_DEFUN([MK_AM_DISABLE_BINARIES], [
  AC_ARG_ENABLE([binaries],
    AS_HELP_STRING([--disable-binaries, skip building of binary programs]),
                   [], [enable_binaries=yes])
  AM_CONDITIONAL([BUILD_BINARIES], [test "$enable_binaries" = "yes"])
])

AC_DEFUN([MK_AM_DISABLE_INTEGRATION_TESTS], [
  AC_ARG_ENABLE([integration-tests],
    AS_HELP_STRING([--disable-integration-tests, skip building of integration tests]),
                   [], [CPPFLAGS="$CPPFLAGS -DENABLE_INTEGRATION_TESTS"])
])

AC_DEFUN([MK_AM_DISABLE_TRACEROUTE], [
  AC_ARG_ENABLE([traceroute],
    AS_HELP_STRING([--disable-traceroute, do not build traceroute]),
                   [], [CPPFLAGS="$CPPFLAGS -DENABLE_TRACEROUTE"])
])

AC_DEFUN([MK_AM_CHECK_LIBC_FUNCS], [
  AC_CHECK_FUNCS([ \
    err \
    errx \
    warn \
    warnx \
    getopt \
    getopt_long \
    getopt_long_only \
    gmtime_r \
    strcasecmp \
    strtonum \
  ])
  AC_CHECK_DECLS([optreset], [], [], [#include <getopt.h>])
])

AC_DEFUN([MK_AM_LIBEVENT], [

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

  AC_CHECK_FUNCS([bufferevent_openssl_set_allow_dirty_shutdown])

  if test "$mk_not_found" = "1"; then
    AC_MSG_WARN([Failed to find dependency: libevent])
    echo "    - to install on Debian: sudo apt-get install libevent-dev"
    echo "    - to install on OSX: brew install libevent"
    echo "    - to compile from sources: ./build/dependency libevent"
    AC_MSG_ERROR([Please, install libevent and run configure again])
  fi
])

AC_DEFUN([MK_AM_GEOIP], [

  AC_ARG_WITH([geoip],
              [AS_HELP_STRING([--with-geoip],
                [GeoIP library @<:@default=check@:>@])
              ],
              [
                CPPFLAGS="$CPPFLAGS -I$withval/include"
                LDFLAGS="$LDFLAGS -L$withval/lib"
              ],
              [])

  mk_not_found=""
  AC_CHECK_HEADERS(GeoIP.h, [], [mk_not_found=1])
  AC_CHECK_LIB(GeoIP, GeoIP_open, [], [mk_not_found=1])

  if test "$mk_not_found" = "1"; then
    AC_MSG_WARN([Failed to find dependency: geoip])
    echo "    - to install on Debian: sudo apt-get install libgeoip-dev"
    echo "    - to install on OSX: brew install libgeoip"
    echo "    - to compile from sources: ./build/dependency geoip"
    AC_MSG_ERROR([Please, install geoip and run configure again])
  fi
])

AC_DEFUN([MK_AM_OPENSSL], [

  AC_ARG_WITH([openssl],
              [AS_HELP_STRING([--with-openssl],
                [SSL toolkit @<:@default=check@:>@])
              ],
              [
                CPPFLAGS="$CPPFLAGS -I$withval/include"
                LDFLAGS="$LDFLAGS -L$withval/lib"
              ],
              [
	        if test -d /usr/local/Cellar/openssl; then
		  AC_MSG_WARN([Using the OpenSSL installed via brew...])
		  mk_openssl_v=`ls /usr/local/Cellar/openssl|tail -n1`
		  mk_openssl_d="/usr/local/Cellar/openssl/$mk_openssl_v"
		  CPPFLAGS="$CPPFLAGS -I$mk_openssl_d/include"
		  LDFLAGS="$LDFLAGS -L$mk_openssl_d/lib"
		fi
	      ])

  mk_not_found=""
  AC_CHECK_HEADERS(openssl/ssl.h, [], [mk_not_found=1])
  AC_CHECK_LIB(crypto, RSA_new, [], [mk_not_found=1])
  AC_CHECK_LIB(ssl, SSL_new, [], [mk_not_found=1])

  dnl This test breaks the build with 12.04 on travis because the linker there
  dnl requires `LD_RUN_PATH` which sadly is not honoured by this test, still
  dnl no worries because actually this check only makes sense for Mac systems
  if test "`uname`" = "Darwin"; then
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
      if test -d /usr/local/Cellar/openssl; then
        AC_MSG_WARN([MacOS ships an old system-wide OpenSSL but you seem to])
        AC_MSG_WARN([have a new version installed with brew.])
        AC_MSG_WARN([So, you should try adding this flag to configure:])
        AC_MSG_WARN(['--with-openssl=/usr/local/Cellar/openssl/VERSION/'])
      fi
      mk_not_found=1
    ],
    [AC_MSG_RESULT([Skip the test because we are cross-compiling])])
  fi

  if test "$mk_not_found" = "1"; then
    AC_MSG_WARN([Failed to find dependency: openssl])
    echo "    - to install on Debian: sudo apt-get install libssl-dev"
    echo "    - to install on OSX: brew install openssl"
    echo "    - to compile from sources: ./build/dependency libressl"
    AC_MSG_ERROR([Please, install openssl and run configure again])
  fi
])

AC_DEFUN([MK_AM_REQUIRE_C99], [
  AC_PROG_CC_C99
  if test x"$ac_cv_prog_cc_c99" = xno; then
    AC_MSG_ERROR([a C99 compiler is required])
  fi
])

AC_DEFUN([MK_AM_REQUIRE_CXX14], [
  mk_saved_cxxflags="$CXXFLAGS"
  CXXFLAGS=-std=c++14
  AC_MSG_CHECKING([whether CXX supports -std=c++14])
  AC_LANG_PUSH([C++])
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([])],
    [AC_MSG_RESULT([yes])]
    [],
    [
     AC_MSG_RESULT([no])
     AC_MSG_ERROR([a C++14 compiler is required])
    ])
  CXXFLAGS="$mk_saved_cxxflags -std=c++14"
  AC_LANG_POP([C++])
])

AC_DEFUN([MK_CHECK_CA_BUNDLE], [
  AC_MSG_CHECKING([CA bundle path])

  AC_ARG_WITH([ca-bundle],
              AC_HELP_STRING([--with-ca-bundle=FILE],
               [Path to a file containing CA certificates (example: /etc/ca-bundle.crt)]),
              [
               want_ca="$withval"
              ],
              [want_ca="unset"])
  
  if test "x$want_ca" != "xunset"; then
    ca="$want_ca"
  else
    ca="no"
    if test "x$cross_compiling" != "xyes"; then
        for a in /etc/ssl/certs/ca-certificates.crt \
                 /etc/pki/tls/certs/ca-bundle.crt \
                 /usr/share/ssl/certs/ca-bundle.crt \
                 /usr/local/share/certs/ca-root.crt \
                 /etc/ssl/cert.pem \
                 /usr/local/etc/openssl/cert.pem; do
          if test -f "$a"; then
            ca="$a"
            break
          fi
        done
    fi
  fi

  if test "x$ca" != "xno"; then
    MK_CA_BUNDLE="$ca"
    AC_DEFINE_UNQUOTED(MK_CA_BUNDLE, "$ca", [Location of default ca bundle])
    AC_SUBST(MK_CA_BUNDLE)
    AC_MSG_RESULT([$ca])
  elif test "x$cross_compiling" == "xyes"; then
    AC_MSG_RESULT([skipped (cross compiling)])
    AC_MSG_WARN([skipped the ca-bundle detection when cross-compiling])
    AC_DEFINE_UNQUOTED(MK_CA_BUNDLE, "", [Location of default ca bundle])
    AC_SUBST(MK_CA_BUNDLE)
  else
    AC_MSG_RESULT([no])
    AC_MSG_ERROR([you should give a ca-bundle location])
  fi
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
  echo "CPP      : $CPP"
  echo "CC       : $CC"
  echo "CXX      : $CXX"
  echo "CFLAGS   : $CFLAGS"
  echo "CPPFLAGS : $CPPFLAGS"
  echo "CXXFLAGS : $CXXFLAGS"
  echo "LDFLAGS  : $LDFLAGS"
  echo "LIBS     : $LIBS"
])
