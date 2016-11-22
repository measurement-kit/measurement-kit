# Part of measurement-kit <https://measurement-kit.github.io/>.
# Measurement-kit is free software. See AUTHORS and LICENSE for more
# information on the copying conditions.

AUTOMAKE_OPTIONS = foreign 1.9 subdir-objects
ACLOCAL_AMFLAGS = -I m4

LIBMEASUREMENT_KIT_I_FLAGS = -I $(top_srcdir)/include
LIBMEASUREMENT_KIT_W_FLAGS = -Wall -Wextra -pedantic

VERSION_INFO = -release @VERSION@ -version-info 0
AM_CFLAGS = $(LIBMEASUREMENT_KIT_W_FLAGS) $(LIBMEASUREMENT_KIT_I_FLAGS)
AM_CXXFLAGS = $(LIBMEASUREMENT_KIT_W_FLAGS) $(LIBMEASUREMENT_KIT_I_FLAGS)

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

VALGRIND_TRAVIS = $(VALGRIND) --suppressions=build/valgrind/travis.supp

run-valgrind-internal:
	test -n "$(LOG_COMPILER)" && {                                         \
	    echo "FATAL: LOG_COMPILER is not set." 1>&2;                       \
	    exit 1;                                                            \
	}
	$(MAKE) $(AM_MAKEFLAGS) check-am || {                                  \
	    if test -f ./test-suite.log; then                                  \
	        cat ./test-suite.log;                                          \
	    fi;                                                                \
	    exit 1;                                                            \
	}
check-am-valgrind:
	$(MAKE) $(AM_MAKEFLAGS) check-am LOG_COMPILER='$(VALGRIND_TRAVIS)'
run-valgrind-travis: check-am-valgrind
run-valgrind:
	$(MAKE) $(AM_MAKEFLAGS) check-am LOG_COMPILER='$(VALGRIND)'
