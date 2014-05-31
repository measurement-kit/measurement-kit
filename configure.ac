# configure.ac

#
# Copyright (c) 2014, Simone Basso, Arturo Filastò, Centro Nexa
#   su Internet & Società, Politecnico di Torino (DAUIN).
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#
#  2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer
#    in the documentation and/or other materials provided with the
#    distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
# IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
# THE POSSIBILITY OF SUCH DAMAGE.
#

# Autoconf requirements
AC_INIT(libight, 4.0.0, bassosimone@gmail.com)

# information on the package
AC_CONFIG_SRCDIR([src/ight_wrappers.cpp])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES])
LT_INIT
AC_CONFIG_HEADERS([config.h])

AC_CANONICAL_HOST

# checks for programs
AC_PROG_INSTALL
AC_PROG_CXX([clang++ g++ c++])
AM_PATH_PYTHON([2.6])

# checks for libraries

AC_ARG_WITH([libevent],
            [AS_HELP_STRING([--with-libevent],
             [event I/O library @<:@default=check@:>@])
            ], [
              #if ! test $withval/include/event2/event.h; then
              #    AC_MSG_ERROR([unable to find event2/event.h])
              #fi
              CPPFLAGS="$CPPFLAGS -I$withval/include"
              LDFLAGS="$LDFLAGS -L$withval/lib"
            ], [])
AC_CHECK_HEADERS(event2/event.h, [],
  AC_MSG_ERROR([unable to find libevent headers]))
AC_CHECK_LIB(event, event_new, [],
  AC_MSG_ERROR([unable to find libevent library]))

# checks for header files
# checks for types
# checks for structures
# checks for compiler characteristics
# checks for library functions
# checks for system services

AC_CONFIG_FILES([Makefile])

AC_OUTPUT

./config.local
