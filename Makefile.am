# Part of measurement-kit <https://measurement-kit.github.io/>.
# Measurement-kit is free software. See AUTHORS and LICENSE for more
# information on the copying conditions.

AUTOMAKE_OPTIONS = foreign 1.9 subdir-objects
ACLOCAL_AMFLAGS  = -I m4

VERSION_INFO = 0:1:0

AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/src
AM_CFLAGS   = -Wall -Wextra -pedantic

lib_LTLIBRARIES               = libmeasurement_kit.la
libmeasurement_kit_la_LDFLAGS = -version-info $(VERSION_INFO)
libmeasurement_kit_la_SOURCES = # Empty

noinst_PROGRAMS    = # Empty
ALL_TESTS          = # Empty

include Makefile-gen.am

TESTS = $(ALL_TESTS)
check_PROGRAMS = $(ALL_TESTS)

VALGRIND = valgrind --trace-children=yes --error-exitcode=1 \
                    --dsymutil=yes --leak-check=yes

run-valgrind: $(check_PROGRAMS)
	for test_name in $(TESTS); do       \
	  $(VALGRIND) $$test_name || exit 1;\
	done
