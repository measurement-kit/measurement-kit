/* libneubot/utils.c */

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

#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <event.h>

#include "strtonum.h"
#include "utils.h"

/* Apparently, this is needed to compile on Android */
#ifndef in_port_t
# define in_port_t uint16_t
#endif

void
neubot_timeval_now(struct timeval *tv)
{
	(void) gettimeofday(tv, NULL);
}

double
neubot_time_now(void)
{
	struct timeval tv;
	double result;

	(void) gettimeofday(&tv, NULL);
	result = tv.tv_sec + tv.tv_usec / (double) 1000000.0;

	return (result);
}

int
neubot_listen(int use_ipv6, const char *address, const char *port)
{
	struct sockaddr_storage storage;
	struct sockaddr_in *sin;
	struct sockaddr_in6 *sin6;
	struct sockaddr *sa;
	socklen_t salen;

	const char *errstr;
	int activate;
	int family;
	int filedesc;
	int result;

	if (use_ipv6)
		family = AF_INET6;
	else
		family = AF_INET;

	filedesc = socket(family, SOCK_STREAM, 0);
	if (filedesc == -1)
		return (-1);

	result = evutil_make_socket_nonblocking(filedesc);
	if (result != 0)
		goto cleanup;

	activate = 1;
	result = setsockopt(filedesc, SOL_SOCKET, SO_REUSEADDR,
	    &activate, sizeof(activate));
	if (result != 0)
		goto cleanup;

	memset(&storage, 0, sizeof(storage));
	if (use_ipv6) {
		sin6 = (struct sockaddr_in6 *) &storage;
		sin6->sin6_family = AF_INET6;
		result = inet_pton(AF_INET6, address, &sin6->sin6_addr);
		if (result != 1)
			goto cleanup;
		sin6->sin6_port = (in_port_t) neubot_strtonum(port, 0,
		    65535, &errstr);
		if (errstr)
			goto cleanup;
		sin6->sin6_port = htons(sin6->sin6_port);
		salen = sizeof(struct sockaddr_in6);
	} else {
		sin = (struct sockaddr_in *) &storage;
		sin->sin_family = AF_INET;
		result = inet_pton(AF_INET, address, &sin->sin_addr);
		if (result != 1)
			goto cleanup;
		sin->sin_port = (in_port_t) neubot_strtonum(port, 0,
		    65535, &errstr);
		if (errstr)
			goto cleanup;
		sin->sin_port = htons(sin->sin_port);
		salen = sizeof(struct sockaddr_in);
	}
	sa = (struct sockaddr *) &storage;

	result = bind(filedesc, sa, salen);
	if (result != 0)
		goto cleanup;

	result = listen(filedesc, 10);
	if (result != 0)
		goto cleanup;

	return (filedesc);

      cleanup:
	(void) close(filedesc);
	return (-1);
}

void
neubot_xfree(void *ptr)
{
	if (ptr != NULL)
		free(ptr);
}

void
neubot_timeval_init(struct timeval *tv, double delta)
{
        tv->tv_sec = (time_t) floor(delta);
        tv->tv_usec = (suseconds_t) ((delta - floor(delta)) * 1000000);
}
