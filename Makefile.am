# Makefile.am

# Part of measurement-kit <https://measurement-kit.github.io/>.
# Measurement-kit is free software. See AUTHORS and LICENSE for more
# information on the copying conditions.

AUTOMAKE_OPTIONS = foreign 1.9 subdir-objects
ACLOCAL_AMFLAGS = -I m4

LIBMEASUREMENT_KIT_I_FLAGS = -I $(top_srcdir)/include
LIBMEASUREMENT_KIT_I_FLAGS += -I $(top_srcdir)/src

DIST_SUBDIRS =  # Empty
SUBDIRS =  # Empty

VERSION_INFO = 0:1:0
AM_CFLAGS = -pthread $(LIBMEASUREMENT_KIT_I_FLAGS)
AM_CXXFLAGS = -pthread $(LIBMEASUREMENT_KIT_I_FLAGS)

lib_LTLIBRARIES =  # Empty
libmeasurement_kit_la_LDFLAGS = -pthread -version-info $(VERSION_INFO) -lyaml-cpp
libmeasurement_kit_la_SOURCES =  # Empty

noinst_LTLIBRARIES =  # Empty
noinst_PROGRAMS =  # Empty

include src/include.am
include example/include.am
include test/include.am

lib_LTLIBRARIES += libmeasurement_kit.la

VALGRIND = valgrind --trace-children=yes --error-exitcode=1 \
                    --dsymutil=yes --leak-check=yes

run-valgrind: $(check_PROGRAMS)
	for test_name in $(TESTS); do       \
	  $(VALGRIND) $$test_name || exit 1;\
	done

if USE_BUILTIN_JANSSON
    DIST_SUBDIRS += third_party/jansson
    SUBDIRS += third_party/jansson
endif

if USE_BUILTIN_LIBMAXMINDDB
    DIST_SUBDIRS += third_party/libmaxminddb
    SUBDIRS += third_party/libmaxminddb
endif

if USE_BUILTIN_LIBEVENT
    DIST_SUBDIRS += third_party/libevent
    SUBDIRS += third_party/libevent
endif
