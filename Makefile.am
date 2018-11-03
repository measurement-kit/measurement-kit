AUTOMAKE_OPTIONS = foreign 1.9 subdir-objects
ACLOCAL_AMFLAGS = -I m4

AM_LIBTOOLFLAGS = --silent

VERSION_INFO = -release @VERSION@ -version-info 0

lib_LTLIBRARIES               = libmeasurement_kit.la
libmeasurement_kit_la_LDFLAGS = $(VERSION_INFO)
libmeasurement_kit_la_SOURCES = # Empty
measurement_kit_SOURCES       = # Empty

if BUILD_BINARIES
    bin_PROGRAMS = measurement_kit
    measurement_kit_LDADD = libmeasurement_kit.la
endif

noinst_PROGRAMS    = # Empty
ALL_TESTS          = # Empty

include include.am

# We link with libresolv to have an oracle for our base64 implementation
if HAVE_RESOLV
    test_common_encoding_LDADD += -lresolv
endif

TESTS = $(ALL_TESTS)
check_PROGRAMS = $(ALL_TESTS)
