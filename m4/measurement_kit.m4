dnl Part of measurement-kit <https://measurement-kit.github.io/>.
dnl Measurement-kit is free software. See AUTHORS and LICENSE for more
dnl information on the copying conditions.

dnl Common macros for mkok projects.

dnl Macro that defines the --disable-example flag
AC_DEFUN([MKIT_ARG_DISABLE_EXAMPLES], [
  AC_ARG_ENABLE([examples],
    AS_HELP_STRING([--disable-examples, skip building of examples programs]),
      [], [enable_examples=yes])
  AM_CONDITIONAL([BUILD_EXAMPLES], [test "$enable_examples" = "yes"])
])

dnl Macro that defines the --enable-tests flag
AC_DEFUN([MKIT_ARG_DISABLE_TESTS], [
  AC_ARG_ENABLE([tests],
    AS_HELP_STRING([--disable-tests, skip building and running test programs]),
      [], [enable_tests=yes])
  AM_CONDITIONAL([BUILD_TESTS], [test "$enable_tests" = "yes"])
])

dnl Macro that defines the --enable-coverage flag
AC_DEFUN([MKIT_ARG_ENABLE_COVERAGE], [
  AC_ARG_ENABLE([coverage],
    AS_HELP_STRING([--enable-coverage, build for coverage]),
      [enable_coverage=yes], [])
])

dnl Find boost and add --with-boost flag
AC_DEFUN([MKIT_FIND_BOOST], [
  MKIT_BOOST=yes
  AC_ARG_WITH([boost],
    AS_HELP_STRING([--with-boost, Boost C++ library]), [
      if test "$withval" != "builtin"; then
        CPPFLAGS="$CPPFLAGS -I$withval/include"
        LDFLAGS="$LDFLAGS -L$withval/lib"
      fi], [])
  AC_LANG_PUSH([C++])
  AC_CHECK_HEADERS(boost/iterator/iterator_adaptor.hpp, [], [MKIT_BOOST=no])
  AC_CHECK_HEADERS(boost/iterator/iterator_facade.hpp, [], [MKIT_BOOST=no])
  AC_CHECK_HEADERS(boost/noncopyable.hpp, [], [MKIT_BOOST=no])
  AC_CHECK_HEADERS(boost/shared_ptr.hpp, [], [MKIT_BOOST=no])
  AC_CHECK_HEADERS(boost/smart_ptr/shared_ptr.hpp, [], [MKIT_BOOST=no])
  AC_CHECK_HEADERS(boost/type_traits.hpp, [], [MKIT_BOOST=no])
  AC_CHECK_HEADERS(boost/utility/enable_if.hpp, [], [MKIT_BOOST=no])
  AC_CHECK_HEADERS(boost/utility.hpp, [], [MKIT_BOOST=no])
  AC_LANG_POP([C++])
])

dnl Find http-parser and add --with-http-parser flag
AC_DEFUN([MKIT_FIND_HTTP_PARSER], [
  MKIT_HTTP_PARSER=yes
  AC_ARG_WITH([http-parser],
    AS_HELP_STRING([--with-http-parser, Node.js HTTP parser library]), [
      if test "$withval" != "builtin"; then
        CPPFLAGS="$CPPFLAGS -I$withval/include"
        LDFLAGS="$LDFLAGS -L$withval/lib"
      fi], [])
  AC_CHECK_HEADERS(http_parser.h, [], [MKIT_HTTP_PARSER=no])
  AC_CHECK_LIB(http_parser, http_parser_init, [], [MKIT_HTTP_PARSER=no])
])

dnl Find jansson and add --with-jansson flag
AC_DEFUN([MKIT_FIND_JANSSON], [
  MKIT_JANSSON=yes
  AC_ARG_WITH([jansson],
    AS_HELP_STRING([--with-jansson, JSON parse/emit library]), [
      if test "$withval" != "builtin"; then
        CPPFLAGS="$CPPFLAGS -I$withval/include"
        LDFLAGS="$LDFLAGS -L$withval/lib"
      fi], [])
  AC_CHECK_HEADERS(jansson.h, [], [MKIT_JANSSON=no])
  AC_CHECK_LIB(jansson, json_null, [], [MKIT_JANSSON=no])
])

dnl Find libevent and add --with-libevent flag
AC_DEFUN([MKIT_FIND_LIBEVENT], [
  MKIT_LIBEVENT=yes
  AC_ARG_WITH([libevent],
    AS_HELP_STRING([--with-libevent, asynchronous I/O library]), [
      if test "$withval" != "builtin"; then
        CPPFLAGS="$CPPFLAGS -I$withval/include"
        LDFLAGS="$LDFLAGS -L$withval/lib"
      fi], [])
  AC_CHECK_HEADERS(event2/event.h, [], [MKIT_LIBEVENT=no])
  AC_CHECK_LIB(event, event_new, [], [MKIT_LIBEVENT=no])
  AC_CHECK_LIB(event_pthreads, evthread_use_pthreads, [], [MKIT_LIBEVENT=no])
  dnl Disable the following because for now we are not using it:
  dnl AC_CHECK_LIB(event_openssl, bufferevent_openssl_filter_new, [],
  dnl   [MKIT_LIBEVENT=no])
])

dnl Find libmaxminddb and add --with-libmaxminddb flag
AC_DEFUN([MKIT_FIND_LIBMAXMINDDB], [
  MKIT_LIBMAXMINDDB=yes
  AC_ARG_WITH([libmaxminddb],
    AS_HELP_STRING([--with-libmaxminddb, MaxMind DB library]), [
      if test "$withval" != "builtin"; then
        CPPFLAGS="$CPPFLAGS -I$withval/include"
        LDFLAGS="$LDFLAGS -L$withval/lib"
      fi], [])
  AC_CHECK_HEADERS(maxminddb.h, [], [MKIT_LIBMAXMINDDB=no])
  AC_CHECK_LIB(maxminddb, MMDB_open, [], [MKIT_LIBMAXMINDDB=no])
])

dnl Find OpenSSL (or LibReSSL) and add --with-openssl flag
AC_DEFUN([MKIT_FIND_OPENSSL], [
  MKIT_OPENSSL=yes
  AC_ARG_WITH([openssl],
    AS_HELP_STRING([--with-openssl, OpenSSL-compatible library]), [
      if test "$withval" != "builtin"; then
        CPPFLAGS="$CPPFLAGS -I$withval/include"
        LDFLAGS="$LDFLAGS -L$withval/lib"
      fi], [])
  AC_CHECK_HEADERS(openssl/ssl.h, [], [MKIT_OPENSSL=no])
  AC_CHECK_LIB(crypto, RSA_new, [], [MKIT_OPENSSL=no])
  AC_CHECK_LIB(ssl, SSL_new, [], [MKIT_OPENSSL=no])
])

dnl Find tor-libs and add --with-tor-libs flag
AC_DEFUN([MKIT_FIND_TOR_LIBS], [
  MKIT_TOR_LIBS=yes
  AC_ARG_WITH([tor-libs],
    AS_HELP_STRING([--with-tor-libs, Libraries containing all Tor sources]), [
      if test "$withval" != "builtin"; then
        CPPFLAGS="$CPPFLAGS -I$withval/include"
        LDFLAGS="$LDFLAGS -L$withval/lib"
      fi], [])
  AC_CHECK_HEADERS(libtor.h, [], [MKIT_TOR_LIBS=no])
  saved_LIBS=$LIBS
  TOR_LIBS_="-ltor -lor-crypto -lkeccak-tiny -led25519_ref10 -led25519_donna"
  TOR_LIBS_="${TOR_LIBS_} -lcurve25519_donna -lor-trunnel -lor-event -lor"
  LIBS="${TOR_LIBS_} $LIBS"
  AC_MSG_CHECKING([whether we can link with Tor])
  AC_LANG_PUSH([C])
  AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM([
#include <libtor.h>
    ], [
const char *args[] = {
  "tor",
  "-h",
  0
};
tor_main(3, (char **)args);
    ])], [AC_MSG_RESULT([yes])],
    [AC_MSG_RESULT([no])
     MKIT_TOR_LIBS=no])
  AC_LANG_POP([C])
  if test "$MKIT_TOR_LIBS" != "yes"; then
    LIBS=$saved_LIBS
  fi
])

dnl Find yaml-cpp and add --with-yaml-cpp flag
AC_DEFUN([MKIT_FIND_YAML_CPP], [
  MKIT_YAML_CPP=yes
  AC_ARG_WITH([yaml-cpp],
    AS_HELP_STRING([--with-yaml-cpp, YAML parser/emitter library]), [
      if test "$withval" != "builtin"; then
        CPPFLAGS="$CPPFLAGS -I$withval/include"
        LDFLAGS="$LDFLAGS -L$withval/lib"
      fi], [])
  AC_LANG_PUSH([C++])
  AC_CHECK_HEADERS(yaml-cpp/yaml.h, [], [MKIT_YAML_CPP=no])
  saved_LIBS=$LIBS
  LIBS="-lyaml-cpp $LIBS"
  AC_MSG_CHECKING([whether we can link with yaml-cpp])
  AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM([
#include <yaml-cpp/yaml.h>
    ], [
YAML::Node node;
node = 3.14;
    ])], [AC_MSG_RESULT([yes])],
    [AC_MSG_RESULT([no])
     MKIT_YAML_CPP=no])
  AC_LANG_POP([C++])
  if test "$MKIT_YAML_CPP" != "yes"; then
    LIBS=$saved_LIBS
  fi
])

dnl Find zlib and add --with-zlib flag
AC_DEFUN([MKIT_FIND_ZLIB], [
  MKIT_ZLIB=yes
  AC_ARG_WITH([zlib],
    AS_HELP_STRING([--with-zlib, The zlib library]), [
      if test "$withval" != "builtin"; then
        CPPFLAGS="$CPPFLAGS -I$withval/include"
        LDFLAGS="$LDFLAGS -L$withval/lib"
      fi], [])
  AC_CHECK_HEADERS(zlib.h, [], [MKIT_ZLIB=no])
  AC_CHECK_LIB(z, inflate, [], [MKIT_ZLIB=no])
])

dnl Make sure that the compiler supports C++11
AC_DEFUN([MKIT_REQUIRE_CXX11], [
  saved_cxxflags="$CXXFLAGS"
  CXXFLAGS=-std=c++11
  AC_MSG_CHECKING([whether CXX supports -std=c++11])
  AC_LANG_PUSH([C++])
  AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM([])], [AC_MSG_RESULT([yes])],
    [AC_MSG_RESULT([no])
     AC_MSG_ERROR([a C++11 compiler is required])])
  CXXFLAGS="$saved_cxxflags -std=c++11"
  AC_LANG_POP([C++])
])

dnl Add as much warnings as possible to CXXFLAGS
AC_DEFUN([MKIT_ADD_WARNINGS_TO_CXXFLAGS], [
  BASE_CXXFLAGS="-Wall -Wextra -pedantic"
  AC_MSG_CHECKING([whether the C++ compiler is clang++])
  if test echo | $CXX -dM -E - | grep __clang__ > /dev/null; then
    AC_MSG_RESULT([yes])
    CXXFLAGS="$CXXFLAGS $BASE_CXXFLAGS -Wmissing-prototypes"
  else
    CXXFLAGS="$CXXFLAGS $BASE_CXXFLAGS"
    AC_MSG_RESULT([no])
  fi
])

dnl Add $(top_srcdir)/include to CPPFLAGS
AC_DEFUN([MKIT_ADD_INCLUDE_TO_CPPFLAGS], [
  CPPFLAGS="$CPPFLAGS -I \$(top_srcdir)/include"
])

dnl If needed, add flags required to enable coverage
AC_DEFUN([MKIT_ADD_COVERAGE_FLAGS_IF_NEEDED], [
  if test "$enable_coverage" = "yes"; then
    CFLAGS="$CFLAGS --coverage"
    CXXFLAGS="$CXXFLAGS --coverage"
    LDFLAGS="$LDFLAGS --coverage"
  fi
])

dnl Print what has been configured by ./configure
AC_DEFUN([MKIT_PRINT_SUMMARY], [
  echo "==== dependencies found ===="
  echo "boost         : $MKIT_BOOST"
  echo "http_parser   : $MKIT_HTTP_PARSER"
  echo "jansson       : $MKIT_JANSSON"
  echo "libevent      : $MKIT_LIBEVENT"
  echo "libmaxminddb  : $MKIT_LIBMAXMINDDB"
  echo "openssl       : $MKIT_OPENSSL"
  echo "tor_libs      : $MKIT_TOR_LIBS"
  echo "yaml-cpp      : $MKIT_YAML_CPP"
  echo "zlib          : $MKIT_ZLIB"
  echo ""
  echo "==== configured flags ===="
  echo "CPPFLAGS      : $CPPFLAGS"
  echo "CXX           : $CXX"
  echo "CXXFLAGS      : $CXXFLAGS"
  echo "LDFLAGS       : $LDFLAGS"
  echo "LIBS          : $LIBS"
  echo "coverage      : $enable_coverage"
  echo "examples      : $enable_examples"
  echo "tests         : $enable_tests"
])
