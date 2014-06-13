# Makefile.am

#
# This file is part of Libight <https://libight.github.io/>.
#
# Libight is free software. See AUTHORS and LICENSE for more
# information on the copying conditions.
#

AUTOMAKE_OPTIONS = foreign 1.9 subdir-objects
ACLOCAL_AMFLAGS = -I m4

VERSION_INFO = 4:0:0
AM_CFLAGS = -Wall -Wmissing-prototypes -Wextra -pedantic -I $(top_srcdir)/src
AM_CXXFLAGS = -Wall -Wmissing-prototypes -Wextra -pedantic -I $(top_srcdir)/src

lib_LTLIBRARIES = libight.la
libight_la_LDFLAGS = -version-info $(VERSION_INFO)
libight_la_SOURCES =

noinst_LTLIBRARIES =

include src/include.am
include test/include.am
