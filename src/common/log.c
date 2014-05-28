/* libneubot/log.c */

/*-
 * Copyright (c) 2013
 *     Nexa Center for Internet & Society, Politecnico di Torino (DAUIN)
 *     and Simone Basso <bassosimone@gmail.com>.
 *
 * This file is part of Neubot <http://www.neubot.org/>.
 *
 * Neubot is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Neubot is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Neubot.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef NEUBOT_ANDROID
#include <android/log.h>
#endif

#include <stdarg.h>
#include <stdio.h>

#include "log.h"

static void
neubot_warnv(const char *fmt, va_list ap)
{
#ifndef NEUBOT_ANDROID
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
#else
	__android_log_vprint(ANDROID_LOG_WARN, "libneubot", fmt, ap);
#endif
}

void
neubot_warn(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	neubot_warnv(fmt, ap);
	va_end(ap);
}

void
neubot_info(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	neubot_warnv(fmt, ap);
	va_end(ap);
}
