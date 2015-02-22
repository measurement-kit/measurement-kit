# Makefile.am

#
# This file is part of Libight <https://libight.github.io/>.
#
# Libight is free software. See AUTHORS and LICENSE for more
# information on the copying conditions.
#

AUTOMAKE_OPTIONS = foreign 1.9 subdir-objects
ACLOCAL_AMFLAGS = -I m4

LIBIGHT_I_FLAGS = -I $(top_srcdir)/include
LIBIGHT_I_FLAGS += -I $(top_srcdir)/src
LIBIGHT_W_FLAGS = -Wall -Wmissing-prototypes -Wextra -pedantic

VERSION_INFO = 4:0:0
AM_CFLAGS = $(LIBIGHT_W_FLAGS) $(LIBIGHT_I_FLAGS)
AM_CXXFLAGS = $(LIBIGHT_W_FLAGS) $(LIBIGHT_I_FLAGS)

lib_LTLIBRARIES =  # Empty
libight_la_LDFLAGS = -version-info $(VERSION_INFO) -lyaml-cpp
libight_la_SOURCES =  # Empty

noinst_LTLIBRARIES =  # Empty

include src/include.am
include test/include.am

lib_LTLIBRARIES += libight.la
