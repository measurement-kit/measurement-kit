AC_INIT(measurement_kit, 0.10.14, bassosimone@gmail.com)

AC_CONFIG_SRCDIR([Makefile.am])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([parallel-tests 1.11])

LT_INIT([disable-shared], [pic-only])
AC_CONFIG_HEADERS([config.h])

AC_CANONICAL_HOST
AC_PROG_INSTALL

case "$host" in
  *-w64-mingw32)
    # Must link with ws2_32 and iphlpapi for if_nametoindex (since libevent 2.1.12)
    LIBS="$LIBS -lws2_32 -liphlpapi"
    # Required to expose inet_pton()
    CPPFLAGS="$CPPFLAGS -D_WIN32_WINNT=0x0600 -D_POSIX_C_SOURCE"
  ;;
esac

AC_PROG_CXX
MK_PTHREAD

AC_CHECK_FUNCS([err errx warn warnx getopt getopt_long getopt_long_only        \
                strcasecmp strtonum])
AC_CHECK_DECLS([optreset], [], [], [#include <getopt.h>])

AC_CHECK_DECLS([gmtime_r], [], [], [#include <time.h>])
if test $ac_have_decl != 1; then
  AC_MSG_ERROR("No support for gmtime_r")
fi

MK_AM_OPENSSL
MK_AM_LIBEVENT
MK_AM_RESOLV
MK_AM_LIBCURL
MK_AM_LIBMAXMINDDB

MK_MAYBE_CA_BUNDLE

CPPFLAGS="-I \$(top_srcdir)/include $CPPFLAGS"

for flag in -Wall -Wextra -pedantic -Wmissing-prototypes; do
  MK_MAYBE_APPEND_CFLAG([$flag])
  MK_MAYBE_APPEND_CXXFLAG([$flag])
done
MK_MAYBE_APPEND_CXXFLAG([-Wno-c++17-extensions]) # Silence clang on macOS

AX_ADD_FORTIFY_SOURCE

# Force compiling the http_parser in non-strict mode. This allows us to test
# partially broken sites like mail.voila.fr that send spaces between the header
# key and the separator (i.e. ':').
CPPFLAGS="$CPPFLAGS -DHTTP_PARSER_STRICT=0"

# Make sure the build is not gonna fail for deprecated stuff.
CPPFLAGS="$CPPFLAGS -DMK_NETTESTS_INTERNAL"

MK_REQUIRE_CFLAG([-std=c11])
MK_REQUIRE_CXX14

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
