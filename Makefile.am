AUTOMAKE_OPTIONS = foreign 1.9 subdir-objects
ACLOCAL_AMFLAGS = -I m4

include src/include.am

VERSION_INFO = 4:0:0
AM_CFLAGS = -Wall -Wextra -I $(top_srcdir)/src

lib_LTLIBRARIES = libight.la
libight_la_LDFLAGS = -version-info $(VERSION_INFO)
libight_la_LIBADD = $(PTHREAD_LIBS)

libight_la_SOURCES =				\
	src/common/log.c			\
	src/common/stringvector.cpp		\
	src/common/utils.c			\
	src/common/poller.c			\
	src/net/protocol.cpp			\
	src/net/echo.c				\
	src/net/connection.cpp			\
	src/neubot/pollable.cpp			\
	src/ight_wrappers.cpp			\
	src/ext/http-parser/http_parser.c	\
	src/ext/strtonum.c

include_HEADERS	=				\
	src/common/utils.h			\
	src/common/stringvector.h		\
	src/common/poller.h			\
	src/common/log.h			\
	src/net/ll2sock.h			\
	src/net/protocol.h			\
	src/net/connection.h			\
	src/ext/http-parser/http_parser.h	\
	src/ext/strtonum.h			\
	src/ight_wrappers.h
