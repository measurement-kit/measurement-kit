/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#include <arpa/inet.h>
#include <netinet/in.h>

#include <algorithm>

#include <errno.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <event2/event.h>

#include "ext/strtonum.h"
#include <ight/common/log.hpp>
#include <ight/common/utils.hpp>

void
ight_timeval_now(struct timeval *tv)
{
	if (gettimeofday(tv, NULL) != 0)
		abort();
}

double
ight_time_now(void)
{
	struct timeval tv;
	double result;

	ight_timeval_now(&tv);
	result = tv.tv_sec + tv.tv_usec / (double) 1000000.0;

	return (result);
}

evutil_socket_t
ight_listen(int use_ipv6, const char *address, const char *port)
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

	result = ight_storage_init(&storage, &salen, family, address, port);
	if (result == -1)
		return (-1);

	filedesc = ight_socket_create(storage.ss_family, SOCK_STREAM, 0);
	if (filedesc == IGHT_SOCKET_INVALID)
		return (-1);

	result = ight_socket_listen(filedesc, &storage, salen);
	if (result != 0) {
		(void) evutil_closesocket(filedesc);
		return (-1);
	}

	return (filedesc);
}

/* Many system's free() handle NULL; is this needed? */
void
ight_xfree(void *ptr)
{
	if (ptr != NULL)
		free(ptr);
}

struct timeval *
ight_timeval_init(struct timeval *tv, double delta)
{
	ight_info("utils:ight_timeval_init - enter");

	if (delta < 0) {
		ight_info("utils:ight_timeval_init - no init needed");
		return (NULL);
	}
	tv->tv_sec = (time_t) floor(delta);
	tv->tv_usec = (suseconds_t) ((delta - floor(delta)) * 1000000);

	ight_info("utils:ight_timeval_init - ok");
	return (tv);
}

int
ight_storage_init(struct sockaddr_storage *storage, socklen_t *salen,
    const char *family, const char *address, const char *port)
{
	int _family, _port, result;
	const char *errstr;

	ight_info("utils:ight_storage_init - enter");

	/* TODO: support also AF_INET, AF_INET6, ... */
	if (strcmp(family, "PF_INET") == 0) {
		_family = PF_INET;
	} else if (strcmp(family, "PF_INET6") == 0) {
		_family = PF_INET6;
	} else {
		ight_warn("utils:ight_storage_init: invalid family");
		return (-1);
	}

	_port = (int) ight_strtonum(port, 0, 65535, &errstr);
	if (errstr != NULL) {
		ight_warn("utils:ight_storage_init: invalid port");
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
		       ight_warn("utils:ight_storage_init: invalid addr");
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
		      ight_warn("utils:ight_storage_init: invalid addr");
		      return (-1);
	      }
	      *salen = sizeof (struct sockaddr_in);
	      break;
	}

	default:
		abort();
	}

	ight_info("utils:ight_storage_init - ok");
	return (0);
}

evutil_socket_t
ight_socket_create(int domain, int type, int protocol)
{
	evutil_socket_t filedesc;
	int result;

	ight_info("utils:ight_socket - enter");

	filedesc = socket(domain, type, protocol);
	if (filedesc == IGHT_SOCKET_INVALID) {
		ight_warn("utils:ight_socket: cannot create socket");
		return (IGHT_SOCKET_INVALID);
	}

	result = evutil_make_socket_nonblocking(filedesc);
	if (result != 0) {
		ight_warn("utils:ight_socket: cannot make nonblocking");
		(void) evutil_closesocket(filedesc);
		return (IGHT_SOCKET_INVALID);
	}

	ight_info("utils:ight_socket - ok");
	return (filedesc);
}

int
ight_socket_connect(evutil_socket_t filedesc,
    struct sockaddr_storage *storage, socklen_t salen)
{
	int result;

	ight_info("utils:ight_socket_connect - enter");

	result = connect(filedesc, (struct sockaddr *) storage, salen);
	if (result != 0) {
#ifndef WIN32
		if (errno == EINPROGRESS)
#else
		if (WSAGetLastError() == WSA_EINPROGRESS)  /* untested */
#endif
			goto looksgood;
		ight_warn("utils:ight_socket_connect - connect() failed");
		return (-1);
	}

    looksgood:
	ight_info("utils:ight_socket_connect - ok");
	return (0);
}

int
ight_socket_listen(evutil_socket_t filedesc, struct sockaddr_storage *storage,
    socklen_t salen)
{
	int result, activate;

	ight_info("utils:ight_socket_listen - enter");

	activate = 1;
	result = setsockopt(filedesc, SOL_SOCKET, SO_REUSEADDR,
	    &activate, sizeof(activate));
	if (result != 0) {
		ight_warn("utils:ight_socket_listen - setsockopt() failed");
		return (-1);
	}

	result = bind(filedesc, (struct sockaddr *) storage, salen);
	if (result != 0) {
		ight_warn("utils:ight_socket_listen - bind() failed");
		return (-1);
	}

	result = listen(filedesc, 10);
	if (result != 0) {
		ight_warn("utils:ight_socket_listen - listen() failed");
		return (-1);
	}

	ight_info("utils:ight_socket_listen - ok");
	return (0);
}

// Stolen from:
// http://stackoverflow.com/questions/440133/how-do-i-create-a-random-alpha-numeric-string-in-c
std::string ight_random_str(size_t length)
{
    auto randchar = []() -> char
    {
        const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[rand() % max_index];
    };
    std::string str(length, 0);
    std::generate_n(str.begin(), length, randchar);
    return str;
}

std::string ight_random_str_uppercase(size_t length)
{
    auto randchar = []() -> char
    {
        const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[rand() % max_index];
    };
    std::string str(length, 0);
    std::generate_n(str.begin(), length, randchar);
    return str;
}
