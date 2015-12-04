# Makefile.am

# Part of measurement-kit <https://measurement-kit.github.io/>.
# Measurement-kit is free software. See AUTHORS and LICENSE for more
# information on the copying conditions.

AUTOMAKE_OPTIONS = foreign 1.9 subdir-objects
ACLOCAL_AMFLAGS = -I m4

LIBMEASUREMENT_KIT_I_FLAGS = -I $(top_srcdir)/include
LIBMEASUREMENT_KIT_I_FLAGS += -I $(top_srcdir)/src
LIBMEASUREMENT_KIT_W_FLAGS = -Wall -Wextra -pedantic

VERSION_INFO = 4:0:0
AM_CFLAGS = -pthread $(LIBMEASUREMENT_KIT_W_FLAGS) $(LIBMEASUREMENT_KIT_I_FLAGS)
AM_CXXFLAGS = -pthread $(LIBMEASUREMENT_KIT_W_FLAGS) $(LIBMEASUREMENT_KIT_I_FLAGS)

lib_LTLIBRARIES =  # Empty
libmeasurement_kit_la_LDFLAGS = -pthread -version-info $(VERSION_INFO) -lyaml-cpp
libmeasurement_kit_la_SOURCES =  # Empty

noinst_LTLIBRARIES =  # Empty

include src/include.am
include test/include.am

lib_LTLIBRARIES += libmeasurement-kit.la
