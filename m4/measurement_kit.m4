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

dnl Print instructions to install a package
AC_DEFUN([MKIT_HOWTO_INSTALL], [
    echo ""
    echo "WARNING: $3 is not installed; to install:"
    echo "    - on ubuntu, run: 'sudo apt-get install $2'; or"
    ifelse($1, [], [], [echo "    - on osx, run: 'brew install $1'; or"])
    echo "    - you can install measurement-kit's builtin version:"
    echo "        - run: './build/dependency $3'; then"
    echo "        - pass '--with-$3=builtin' to './configure'"
])

dnl Exit if there are missing dependencies
AC_DEFUN([MKIT_REQUIRE_DEPENDENCIES], [
    SHOULD_EXIT=no
    if test "$MKIT_WHICH_BOOST" = "no"; then
	MKIT_HOWTO_INSTALL([boost], [libboost-dev], [boost])
        SHOULD_EXIT=yes
    fi
    if test "$MKIT_WHICH_CATCH" = "no"; then
	MKIT_HOWTO_INSTALL([], [catch], [Catch])
        SHOULD_EXIT=yes
    fi
    if test "$MKIT_WHICH_HTTP_PARSER" = "no"; then
	MKIT_HOWTO_INSTALL([http-parser], [libhttp-parser-dev], [http-parser])
        SHOULD_EXIT=yes
    fi
    if test "$MKIT_WHICH_JANSSON" = "no"; then
	MKIT_HOWTO_INSTALL([jansson], [libjansson-dev], [jansson])
        SHOULD_EXIT=yes
    fi
    if test "$MKIT_WHICH_LIBEVENT" = "no"; then
	MKIT_HOWTO_INSTALL([libevent], [libevent-dev], [libevent])
        SHOULD_EXIT=yes
    fi
    if test "$MKIT_WHICH_LIBMAXMINDDB" = "no"; then
	MKIT_HOWTO_INSTALL([libmaxminddb], [libmaxminddb-dev], [libmaxminddb])
        SHOULD_EXIT=yes
    fi
    if test "$MKIT_WHICH_YAML_CPP" = "no"; then
	MKIT_HOWTO_INSTALL([yaml-cpp], [libyaml-cpp-dev], [yaml-cpp])
        SHOULD_EXIT=yes
    fi
    if test "$SHOULD_EXIT" != no; then
        echo ""
        AC_MSG_ERROR([Some dependencies are missing, exiting.])
    fi
])

dnl Find boost and add --with-boost flag
AC_DEFUN([MKIT_AM_BOOST], [
  MKIT_WHICH_BOOST=system
  AC_ARG_WITH([boost],
    AS_HELP_STRING([--with-boost, Boost C++ library]), [
      MKIT_WHICH_BOOST=$withval], [])
  saved_CPPFLAGS=$CPPFLAGS
  if test "$MKIT_WHICH_BOOST" != "system"; then
    CPPFLAGS="-I$MKIT_WHICH_BOOST/include $CPPFLAGS"
  fi
  AC_LANG_PUSH([C++])
  AC_CHECK_HEADERS(boost/iterator/iterator_adaptor.hpp, [],
                   [MKIT_WHICH_BOOST=no])
  AC_CHECK_HEADERS(boost/iterator/iterator_facade.hpp, [],
                   [MKIT_WHICH_BOOST=no])
  AC_CHECK_HEADERS(boost/noncopyable.hpp, [], [MKIT_WHICH_BOOST=no])
  AC_CHECK_HEADERS(boost/shared_ptr.hpp, [], [MKIT_WHICH_BOOST=no])
  AC_CHECK_HEADERS(boost/smart_ptr/shared_ptr.hpp, [], [MKIT_WHICH_BOOST=no])
  AC_CHECK_HEADERS(boost/type_traits.hpp, [], [MKIT_WHICH_BOOST=no])
  AC_CHECK_HEADERS(boost/utility/enable_if.hpp, [], [MKIT_WHICH_BOOST=no])
  AC_CHECK_HEADERS(boost/utility.hpp, [], [MKIT_WHICH_BOOST=no])
  AC_LANG_POP([C++])
  if test "$MKIT_WHICH_BOOST" = "no"; then
    CPPFLAGS=$saved_CPPFLAGS
  fi
])

dnl Find Catch and add --with-Catch flag
AC_DEFUN([MKIT_AM_CATCH], [
  MKIT_WHICH_CATCH=system
  AC_ARG_WITH([Catch],
    AS_HELP_STRING([--with-Catch, C++ Automated Test Cases in Headers]), [
      MKIT_WHICH_CATCH=$withval], [])
  saved_CPPFLAGS=$CPPFLAGS
  if test "$MKIT_WHICH_CATCH" != "system"; then
    CPPFLAGS="-I$MKIT_WHICH_CATCH/include $CPPFLAGS"
  fi
  AC_LANG_PUSH([C++])
  AC_CHECK_HEADERS(catch.hpp, [], [MKIT_WHICH_CATCH=no])
  AC_LANG_POP([C++])
  if test "$MKIT_WHICH_CATCH" = "no"; then
    CPPFLAGS=$saved_CPPFLAGS
  fi
])

dnl Find http-parser and add --with-http-parser flag
AC_DEFUN([MKIT_AM_HTTP_PARSER], [
  MKIT_WHICH_HTTP_PARSER=system
  AC_ARG_WITH([http-parser],
    AS_HELP_STRING([--with-http-parser, Node.js HTTP parser library]), [
      MKIT_WHICH_HTTP_PARSER=$withval])
  saved_CPPFLAGS=$CPPFLAGS
  saved_LDFLAGS=$LDFLAGS
  if test "$MKIT_WHICH_HTTP_PARSER" != "system"; then
    CPPFLAGS="-I$MKIT_WHICH_HTTP_PARSER/include $CPPFLAGS"
    LDFLAGS="-L$MKIT_WHICH_HTTP_PARSER/lib $LDFLAGS"
    # since http-parser does not use libtool, on Linux we need to tell the
    # linker its prefix when linking libmeasurement_kit.la
    if test "$(uname)" = "Linux"; then
        LDFLAGS="$LDFLAGS -Wl,-rpath=$MKIT_WHICH_HTTP_PARSER/lib"
    fi
  fi
  AC_CHECK_HEADERS(http_parser.h, [], [MKIT_WHICH_HTTP_PARSER=no])
  AC_CHECK_LIB(http_parser, http_parser_init, [], [MKIT_WHICH_HTTP_PARSER=no])
  if test "$MKIT_WHICH_HTTP_PARSER" = "no"; then
    CPPFLAGS=$saved_CPPFLAGS
    LDFLAGS=$saved_LDFLAGS
  fi
])

dnl Find jansson and add --with-jansson flag
AC_DEFUN([MKIT_AM_JANSSON], [
  MKIT_WHICH_JANSSON=system
  AC_ARG_WITH([jansson],
    AS_HELP_STRING([--with-jansson, JSON parse/emit library]), [
      MKIT_WHICH_JANSSON=$withval])
  saved_CPPFLAGS=$CPPFLAGS
  saved_LDFLAGS=$LDFLAGS
  if test "$MKIT_WHICH_JANSSON" != "system"; then
    CPPFLAGS="-I$MKIT_WHICH_JANSSON/include $CPPFLAGS"
    LDFLAGS="-L$MKIT_WHICH_JANSSON/lib $LDFLAGS"
  fi
  AC_CHECK_HEADERS(jansson.h, [], [MKIT_WHICH_JANSSON=no])
  AC_CHECK_LIB(jansson, json_null, [], [MKIT_WHICH_JANSSON=no])
  if test "$MKIT_WHICH_JANSSON" = "no"; then
    CPPFLAGS=$saved_CPPFLAGS
    LDFLAGS=$saved_LDFLAGS
  fi
])

dnl Find libevent and add --with-libevent flag
AC_DEFUN([MKIT_AM_LIBEVENT], [
  MKIT_WHICH_LIBEVENT=system
  AC_ARG_WITH([libevent],
    AS_HELP_STRING([--with-libevent, asynchronous I/O library]), [
      MKIT_WHICH_LIBEVENT=$withval])
  saved_CPPFLAGS=$CPPFLAGS
  saved_LDFLAGS=$LDFLAGS
  if test "$MKIT_WHICH_LIBEVENT" != "system"; then
    CPPFLAGS="-I$MKIT_WHICH_LIBEVENT/include $CPPFLAGS"
    LDFLAGS="-L$MKIT_WHICH_LIBEVENT/lib $LDFLAGS"
  fi
  AC_CHECK_HEADERS(event2/event.h, [], [MKIT_WHICH_LIBEVENT=no])
  AC_CHECK_LIB(event, event_new, [], [MKIT_WHICH_LIBEVENT=no])
  AC_CHECK_LIB(event_pthreads, evthread_use_pthreads, [],
               [MKIT_WHICH_LIBEVENT=no])
  if test "$MKIT_WHICH_LIBEVENT" = "no"; then
    CPPFLAGS=$saved_CPPFLAGS
    LDFLAGS=$saved_LDFLAGS
  fi
])

dnl Find libmaxminddb and add --with-libmaxminddb flag
AC_DEFUN([MKIT_AM_LIBMAXMINDDB], [
  MKIT_WHICH_LIBMAXMINDDB=system
  AC_ARG_WITH([libmaxminddb],
    AS_HELP_STRING([--with-libmaxminddb, MaxMind DB library]), [
      MKIT_WHICH_LIBMAXMINDDB=$withval])
  saved_CPPFLAGS=$CPPFLAGS
  saved_LDFLAGS=$LDFLAGS
  if test "$MKIT_WHICH_LIBMAXMINDDB" != "system"; then
    CPPFLAGS="-I$MKIT_WHICH_LIBMAXMINDDB/include $CPPFLAGS"
    LDFLAGS="-L$MKIT_WHICH_LIBMAXMINDDB/lib $LDFLAGS"
  fi
  AC_CHECK_HEADERS(maxminddb.h, [], [MKIT_WHICH_LIBMAXMINDDB=no])
  AC_CHECK_LIB(maxminddb, MMDB_open, [], [MKIT_WHICH_LIBMAXMINDDB=no])
  if test "$MKIT_WHICH_LIBMAXMINDDB" = "no"; then
    CPPFLAGS=$saved_CPPFLAGS
    LDFLAGS=$saved_LDFLAGS
  fi
])

dnl Find yaml-cpp and add --with-yaml-cpp flag
AC_DEFUN([MKIT_AM_YAML_CPP], [
  MKIT_WHICH_YAML_CPP=system
  AC_ARG_WITH([yaml-cpp],
    AS_HELP_STRING([--with-yaml-cpp, YAML parser/emitter library]), [
      MKIT_WHICH_YAML_CPP=$withval])
  saved_CPPFLAGS=$CPPFLAGS
  saved_LDFLAGS=$LDFLAGS
  if test "$MKIT_WHICH_YAML_CPP" != "system"; then
    CPPFLAGS="-I$MKIT_WHICH_YAML_CPP/include $CPPFLAGS"
    LDFLAGS="-L$MKIT_WHICH_YAML_CPP/lib $LDFLAGS"
    # since yaml-cpp does not use libtool, on Linux we need to tell the
    # linker its prefix when linking libmeasurement_kit.la
    if test "$(uname)" = "Linux"; then
        LDFLAGS="$LDFLAGS -Wl,-rpath=$MKIT_WHICH_YAML_CPP/lib"
    fi
  fi
  saved_LIBS=$LIBS
  LIBS="-lyaml-cpp $LIBS"
  AC_LANG_PUSH([C++])
  AC_CHECK_HEADERS(yaml-cpp/yaml.h, [], [MKIT_WHICH_YAML_CPP=no])
  AC_MSG_CHECKING([whether we can link with yaml-cpp])
  AC_LINK_IFELSE(
    [AC_LANG_PROGRAM([
#include <yaml-cpp/yaml.h>
    ], [
YAML::Node node;
node = 3.14;
    ])], [AC_MSG_RESULT([yes])],
    [AC_MSG_RESULT([no])
    MKIT_WHICH_YAML_CPP=no])
  AC_LANG_POP([C++])
  if test "$MKIT_WHICH_YAML_CPP" = "no"; then
    CPPFLAGS=$saved_CPPFLAGS
    LDFLAGS=$saved_LDFLAGS
    LIBS=$saved_LIBS
  fi
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
  echo "==== dependencies ===="
  echo "boost         : $MKIT_WHICH_BOOST"
  echo "Catch         : $MKIT_WHICH_CATCH"
  echo "jansson       : $MKIT_WHICH_JANSSON"
  echo "http-parser   : $MKIT_WHICH_HTTP_PARSER"
  echo "libevent      : $MKIT_WHICH_LIBEVENT"
  echo "libmaxminddb  : $MKIT_WHICH_LIBMAXMINDDB"
  echo "yaml-cpp      : $MKIT_WHICH_YAML_CPP"
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
