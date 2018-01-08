AUTOMAKE_OPTIONS = foreign 1.9 subdir-objects
ACLOCAL_AMFLAGS = -I m4

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
test_common_encoding_LDADD += -lresolv

TESTS = $(ALL_TESTS)
check_PROGRAMS = $(ALL_TESTS)

#
# Running tests through Valgrind by abusing of the LOG_COMPILER feature
# which is enabled by automake < 1.14 only when parallel tests are forced,
# so you now see why they are enabled explicitly in configure.ac
#
# See http://ansuz.sooke.bc.ca/entry/233
#
# (Yes, of course this means valgrind runs in parallel -- i.e. faster.)
#
# On travis-ci we need on specific suppression for a memory warning that I
# honestly don't understand but that doesn't appear when building using docker
# on circle-ci and gitlab (BTW docker on travis using valgrind was odd since
# there were tons of errors unlike seen on every other docker and so I decided
# it was wiser avoding docker on travis). On other platforms (including my
# own docker on VoidLinux) no suppression file is needed.
#
# See https://travis-ci.org/measurement-kit/measurement-kit/jobs/172742411#L2131
#

VALGRIND = $(LIBTOOL) --mode=execute valgrind --trace-children=yes             \
  --error-exitcode=1 --dsymutil=yes --leak-check=yes --gen-suppressions=all

VALGRIND_TRAVIS = $(VALGRIND) --suppressions=build/valgrind/openssl.supp
VALGRIND_DOCKER = $(VALGRIND) --suppressions=build/valgrind/strlen.supp

check-am-valgrind:
	$(MAKE) $(AM_MAKEFLAGS) check-am LOG_COMPILER='$(VALGRIND_TRAVIS)'
run-valgrind-docker:
	$(MAKE) $(AM_MAKEFLAGS) check-am LOG_COMPILER='$(VALGRIND_DOCKER)'
run-valgrind:
	$(MAKE) $(AM_MAKEFLAGS) check-am LOG_COMPILER='$(VALGRIND)'
