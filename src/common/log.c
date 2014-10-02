/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#ifdef IGHT_ANDROID
#include <android/log.h>
#endif

#include <stdarg.h>
#include <stdio.h>

#include "common/log.h"

static int IGHT_VERBOSE = 0;

static void
ight_warnv(const char *fmt, va_list ap)
{
#ifndef IGHT_ANDROID
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
#else
	__android_log_vprint(ANDROID_LOG_WARN, "libight", fmt, ap);
#endif
}

void
ight_warn(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	ight_warnv(fmt, ap);
	va_end(ap);
}

void
ight_info(const char *fmt, ...)
{
	if (!IGHT_VERBOSE)
		return;

	va_list ap;

	va_start(ap, fmt);
	ight_warnv(fmt, ap);
	va_end(ap);
}

void
ight_set_verbose(int v)
{
	IGHT_VERBOSE = v;
}
