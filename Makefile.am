# Makefile.am

#
# This file is part of Libight <https://libight.github.io/>.
#
# Libight is free software. See AUTHORS and LICENSE for more
# information on the copying conditions.
#

AUTOMAKE_OPTIONS = foreign 1.9 subdir-objects
ACLOCAL_AMFLAGS = -I m4

LIBIGHT_I_FLAGS = -I $(top_srcdir)/src \
	-I $(top_srcdir)/src/ext/yaml-cpp/include \
	-I $(top_srcdir)/src/ext/boost/mpl/include \
	-I $(top_srcdir)/src/ext/boost/preprocessor/include \
	-I $(top_srcdir)/src/ext/boost/smart_ptr/include \
	-I $(top_srcdir)/src/ext/boost/detail/include \
	-I $(top_srcdir)/src/ext/boost/predef/include \
	-I $(top_srcdir)/src/ext/boost/utility/include \
	-I $(top_srcdir)/src/ext/boost/assert/include \
	-I $(top_srcdir)/src/ext/boost/throw_exception/include \
	-I $(top_srcdir)/src/ext/boost/type_traits/include \
	-I $(top_srcdir)/src/ext/boost/config/include \
	-I $(top_srcdir)/src/ext/boost/static_assert/include \
	-I $(top_srcdir)/src/ext/boost/typeof/include \
	-I $(top_srcdir)/src/ext/boost/iterator/include \
	-I $(top_srcdir)/src/ext/boost/core/include

LIBIGHT_W_FLAGS = -Wall -Wmissing-prototypes -Wextra -pedantic

VERSION_INFO = 4:0:0
AM_CFLAGS = $(LIBIGHT_W_FLAGS) $(LIBIGHT_I_FLAGS)
AM_CXXFLAGS = $(LIBIGHT_W_FLAGS) $(LIBIGHT_I_FLAGS)

lib_LTLIBRARIES = libight.la
libight_la_LDFLAGS = -version-info $(VERSION_INFO)
libight_la_SOURCES =

noinst_LTLIBRARIES =

include src/include.am
include test/include.am
