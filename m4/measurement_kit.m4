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
AC_DEFUN([MKIT_AM_BOOST], [
  MKIT_WHICH_BOOST=system
  AC_ARG_WITH([boost],
    AS_HELP_STRING([--with-boost, Boost C++ library]), [
      MKIT_WHICH_BOOST=$withval], [])
  if test "$MKIT_WHICH_BOOST" != "builtin"; then
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
      MKIT_WHICH_BOOST=builtin
    fi
  fi
  if test "$MKIT_WHICH_BOOST" = "builtin"; then
    AC_MSG_WARN([Using builtin boost])
    CPPFLAGS="$CPPFLAGS -I\$(top_srcdir)/third_party/boost/assert/include"
    CPPFLAGS="$CPPFLAGS -I\$(top_srcdir)/third_party/boost/config/include"
    CPPFLAGS="$CPPFLAGS -I\$(top_srcdir)/third_party/boost/core/include"
    CPPFLAGS="$CPPFLAGS -I\$(top_srcdir)/third_party/boost/detail/include"
    CPPFLAGS="$CPPFLAGS -I\$(top_srcdir)/third_party/boost/iterator/include"
    CPPFLAGS="$CPPFLAGS -I\$(top_srcdir)/third_party/boost/mpl/include"
    CPPFLAGS="$CPPFLAGS -I\$(top_srcdir)/third_party/boost/predef/include"
    CPPFLAGS="$CPPFLAGS -I\$(top_srcdir)/third_party/boost/preprocessor/include"
    CPPFLAGS="$CPPFLAGS -I\$(top_srcdir)/third_party/boost/smart_ptr/include"
    CPPFLAGS="$CPPFLAGS -I\$(top_srcdir)/third_party/boost/static_assert/include"
    CPPFLAGS="$CPPFLAGS -I\$(top_srcdir)/third_party/boost/throw_exception/include"
    CPPFLAGS="$CPPFLAGS -I\$(top_srcdir)/third_party/boost/type_traits/include"
    CPPFLAGS="$CPPFLAGS -I\$(top_srcdir)/third_party/boost/typeof/include"
    CPPFLAGS="$CPPFLAGS -I\$(top_srcdir)/third_party/boost/utility/include"
    AC_CONFIG_FILES([third_party/boost/configure])
    AC_CONFIG_SUBDIRS([third_party/boost])
  fi
])

dnl Find jansson and add --with-jansson flag
AC_DEFUN([MKIT_AM_JANSSON], [
  MKIT_WHICH_JANSSON=system
  AC_ARG_WITH([jansson],
    AS_HELP_STRING([--with-jansson, JSON parse/emit library]), [
      MKIT_WHICH_JANSSON=$withval])
  if test "$MKIT_WHICH_JANSSON" != "builtin"; then
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
      MKIT_WHICH_JANSSON=builtin
    fi
  fi
  if test "$MKIT_WHICH_JANSSON" = "builtin"; then
    AC_MSG_WARN([Using builtin jansson])
    CPPFLAGS="$CPPFLAGS -I\$(top_srcdir)/third_party/jansson/jansson/src"
    LDFLAGS="$LDFLAGS -L\$(top_builddir)/third_party/jansson/jansson/src"
    LIBS="$LIBS -ljansson"
    AC_CONFIG_FILES([third_party/jansson/Makefile
                     third_party/jansson/configure])
    AC_CONFIG_SUBDIRS([third_party/jansson])
  fi
  AM_CONDITIONAL([USE_BUILTIN_JANSSON],
                 [test "$MKIT_WHICH_JANSSON" = "builtin"])
])

dnl Find libevent and add --with-libevent flag
AC_DEFUN([MKIT_AM_LIBEVENT], [
  MKIT_WHICH_LIBEVENT=system
  AC_ARG_WITH([libevent],
    AS_HELP_STRING([--with-libevent, asynchronous I/O library]), [
      MKIT_WHICH_LIBEVENT=$withval])
  if test "$MKIT_WHICH_LIBEVENT" != "builtin"; then
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
      MKIT_WHICH_LIBEVENT=builtin
    fi
  fi
  if test "$MKIT_WHICH_LIBEVENT" = "builtin"; then
    AC_MSG_WARN([Using builtin libevent])
    CPPFLAGS="$CPPFLAGS -I\$(top_srcdir)/third_party/libevent/libevent/include"
    CPPFLAGS="$CPPFLAGS -I\$(top_builddir)/third_party/libevent/libevent/include"
    LDFLAGS="$LDFLAGS -L\$(top_builddir)/third_party/libevent/libevent"
    LIBS="$LIBS -levent -levent_pthreads"
    AC_CONFIG_FILES([third_party/libevent/Makefile
                     third_party/libevent/configure])
    AC_CONFIG_SUBDIRS([third_party/libevent])
  fi
  AM_CONDITIONAL([USE_BUILTIN_LIBEVENT],
                 [test "$MKIT_WHICH_LIBEVENT" = "builtin"])
])

dnl Find libmaxminddb and add --with-libmaxminddb flag
AC_DEFUN([MKIT_AM_LIBMAXMINDDB], [
  MKIT_WHICH_LIBMAXMINDDB=system
  AC_ARG_WITH([libmaxminddb],
    AS_HELP_STRING([--with-libmaxminddb, MaxMind DB library]), [
      MKIT_WHICH_LIBMAXMINDDB=$withval])
  if test "$MKIT_WHICH_LIBMAXMINDDB" != "builtin"; then
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
      MKIT_WHICH_LIBMAXMINDDB=builtin
    fi
  fi
  if test $MKIT_WHICH_LIBMAXMINDDB = "builtin"; then
    AC_MSG_WARN([Using builtin libmaxminddb])
    CPPFLAGS="$CPPFLAGS -I\$(top_srcdir)/third_party/libmaxminddb/libmaxminddb/include"
    LDFLAGS="$LDFLAGS -L\$(top_builddir)/third_party/libmaxminddb/libmaxminddb/src"
    LIBS="$LIBS -lmaxminddb"
    AC_CONFIG_FILES([third_party/libmaxminddb/Makefile
                     third_party/libmaxminddb/configure])
    AC_CONFIG_SUBDIRS([third_party/libmaxminddb])
  fi
  AM_CONDITIONAL([USE_BUILTIN_LIBMAXMINDDB],
                 [test "$MKIT_WHICH_LIBMAXMINDDB" = "builtin"])
])

dnl Find yaml-cpp and add --with-yaml-cpp flag
AC_DEFUN([MKIT_AM_YAML_CPP], [
  MKIT_WHICH_YAML_CPP=system
  AC_ARG_WITH([yaml-cpp],
    AS_HELP_STRING([--with-yaml-cpp, YAML parser/emitter library]), [
      MKIT_WHICH_YAML_CPP=$withval])
  if test "$MKIT_WHICH_YAML_CPP" != "builtin"; then
    saved_CPPFLAGS=$CPPFLAGS
    saved_LDFLAGS=$LDFLAGS
    if test "$MKIT_WHICH_YAML_CPP" != "system"; then
      CPPFLAGS="-I$MKIT_WHICH_YAML_CPP/include $CPPFLAGS"
      LDFLAGS="-L$MKIT_WHICH_YAML_CPP/lib $LDFLAGS"
    fi
    saved_LIBS=$LIBS
    LIBS="-lyaml-cpp $LIBS"
    AC_LANG_PUSH([C++])
    AC_CHECK_HEADERS(yaml-cpp/yaml.h, [], [MKIT_WHICH_YAML_CPP=no])
    AC_MSG_CHECKING([whether we can link with yaml-cpp])
    dnl XXX link
    AC_COMPILE_IFELSE(
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
      MKIT_WHICH_YAML_CPP=builtin
    fi
  fi
  if test "$MKIT_WHICH_YAML_CPP" = "builtin"; then
    AC_MSG_WARN([Using builtin yaml-cpp])
    CPPFLAGS="$CPPFLAGS -I\$(top_srcdir)/src/ext/yaml-cpp/include"
    LDFLAGS="$LDFLAGS -L\$(top_builddir)/src/ext/"
    dnl XXX here we should also link when we will use cmake
  fi
  AM_CONDITIONAL([USE_BUILTIN_YAMLCPP],
                 [test "$MKIT_WHICH_YAML_CPP" = "builtin"])
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
  echo "boost         : $MKIT_WHICH_BOOST"
  echo "jansson       : $MKIT_WHICH_JANSSON"
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
