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

#include <limits.h>
#include <errno.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <event2/event.h>

#include "ll2sock.h"
#include "log.h"
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

evutil_socket_t
neubot_listen(int use_ipv6, const char *address, const char *port)
{
	struct sockaddr_storage storage;
	socklen_t salen;
	const char *family;
	evutil_socket_t filedesc;
	int result;

	if (use_ipv6)
		family = "PF_INET6";
	else
		family = "PF_INET";

	result = neubot_storage_init(&storage, &salen, family, address, port);
	if (result == -1)
		return (-1);

	filedesc = neubot_socket_create(storage.ss_family, SOCK_STREAM, 0);
	if (filedesc == NEUBOT_SOCKET_INVALID)
		return (-1);

	result = neubot_socket_listen(filedesc, &storage, salen);
	if (result != 0) {
		(void) evutil_closesocket(filedesc);
		return (-1);
	}

	return (filedesc);
}

void
neubot_xfree(void *ptr)
{
	if (ptr != NULL)
		free(ptr);
}

struct timeval *
neubot_timeval_init(struct timeval *tv, double delta)
{
	neubot_info("utils:neubot_timeval_init - enter");

	if (delta < 0) {
		neubot_info("utils:neubot_timeval_init - no init needed");
		return (NULL);
	}
	tv->tv_sec = (time_t) floor(delta);
	tv->tv_usec = (suseconds_t) ((delta - floor(delta)) * 1000000);

	neubot_info("utils:neubot_timeval_init - ok");
	return (tv);
}

int
neubot_storage_init(struct sockaddr_storage *storage, socklen_t *salen,
    const char *family, const char *address, const char *port)
{
	int _family, _port, result;
	const char *errstr;

	neubot_info("utils:neubot_storage_init - enter");

	if (strcmp(family, "PF_INET") == 0) {
		_family = PF_INET;
	} else if (strcmp(family, "PF_INET6") == 0) {
		_family = PF_INET6;
	} else {
		neubot_warn("utils:neubot_storage_init: invalid family");
		return (-1);
	}

	_port = (int) neubot_strtonum(port, 0, 65535, &errstr);
	if (errstr != NULL) {
		neubot_warn("utils:neubot_storage_init: invalid port");
		return (-1);
	}

	memset(storage, 0, sizeof (*storage));
	switch (_family) {

	case PF_INET6: {
	       struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *) storage;
	       sin6->sin6_family = AF_INET6;
	       sin6->sin6_port = htons(_port);
	       result = inet_pton(AF_INET6, address, &sin6->sin6_addr);
	       if (result != 1) {
		       neubot_warn("utils:neubot_storage_init: invalid addr");
		       return (-1);
	       }
	       *salen = sizeof (struct sockaddr_in6);
	       break;
	}

	case PF_INET: {
	      struct sockaddr_in *sin = (struct sockaddr_in *) storage;
	      sin->sin_family = AF_INET;
	      sin->sin_port = htons(_port);
	      result = inet_pton(AF_INET, address, &sin->sin_addr);
	      if (result != 1) {
		      neubot_warn("utils:neubot_storage_init: invalid addr");
		      return (-1);
	      }
	      *salen = sizeof (struct sockaddr_in);
	      break;
	}

	default:
	      neubot_warn("utils:neubot_storage_init: internal error");
	      return (-1);
	}

	neubot_info("utils:neubot_storage_init - ok");
	return (0);
}

evutil_socket_t
neubot_socket_create(int domain, int type, int protocol)
{
	evutil_socket_t filedesc;
	int result;

	neubot_info("utils:neubot_socket - enter");

	filedesc = socket(domain, type, protocol);
	if (filedesc == NEUBOT_SOCKET_INVALID) {
		neubot_warn("utils:neubot_socket: cannot create socket");
		return (NEUBOT_SOCKET_INVALID);
	}

	result = evutil_make_socket_nonblocking(filedesc);
	if (result != 0) {
		neubot_warn("utils:neubot_socket: cannot make nonblocking");
		(void) evutil_closesocket(filedesc);
		return (NEUBOT_SOCKET_INVALID);
	}

	neubot_info("utils:neubot_socket - ok");
	return (filedesc);
}

int
neubot_socket_connect(evutil_socket_t filedesc,
    struct sockaddr_storage *storage, socklen_t salen)
{
	int result;

	neubot_info("utils:neubot_socket_connect - enter");

	result = connect(filedesc, (struct sockaddr *) storage, salen);
	if (result != 0) {
#ifndef WIN32
		if (errno == EINPROGRESS)
#else
		if (WSAGetLastError() == WSA_EINPROGRESS)  /* untested */
#endif
			goto looksgood;
		neubot_warn("utils:neubot_socket_connect - connect() failed");
		return (-1);
	}

    looksgood:
	neubot_info("utils:neubot_socket_connect - ok");
	return (0);
}

int
neubot_socket_listen(evutil_socket_t filedesc, struct sockaddr_storage *storage,
    socklen_t salen)
{
	int result, activate;

	neubot_info("utils:neubot_socket_listen - enter");

	activate = 1;
	result = setsockopt(filedesc, SOL_SOCKET, SO_REUSEADDR,
	    &activate, sizeof(activate));
	if (result != 0) {
		neubot_warn("utils:neubot_socket_listen - setsockopt() failed");
		return (-1);
	}

	result = bind(filedesc, (struct sockaddr *) storage, salen);
	if (result != 0) {
		neubot_warn("utils:neubot_socket_listen - bind() failed");
		return (-1);
	}

	result = listen(filedesc, 10);
	if (result != 0) {
		neubot_warn("utils:neubot_socket_listen - listen() failed");
		return (-1);
	}

	neubot_info("utils:neubot_socket_listen - ok");
	return (0);
}
