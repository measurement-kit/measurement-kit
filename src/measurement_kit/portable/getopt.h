/*
 * Public domain.
 */
#ifndef SRC_MEASUREMENT_KIT_PORTABLE_GETOPT_H
#define SRC_MEASUREMENT_KIT_PORTABLE_GETOPT_H

#if !defined _WIN32 || defined __MINGW32__

/* Mingw provides an implementation of getopt(). */
#if defined __MINGW32__
#define _BSD_SOURCE /* For optreset */
#endif

#include <getopt.h>
#else
#include "src/measurement_kit/portable/_getopt.h"
#endif /* !_WIN32 || __MINGW_32__ */

#endif
