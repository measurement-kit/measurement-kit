/* libneubot/poller.c */

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

#include <arpa/inet.h>
#include <sys/queue.h>

#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#ifndef WIN32
# include <signal.h>
#endif

#include <event.h>

#include <event2/dns.h>
#include <event2/dns_compat.h>

#include "neubot.h"

#include "log.h"
#include "poller.h"
#include "utils.h"

/* Be compatible with older libevents */
#ifndef evutil_socket_t
# ifdef WIN32
#  define evutil_socket_t uintptr_t
# else
#  define evutil_socket_t int
# endif
#endif

/* To convert long long to sockets */
#ifdef WIN32
# define EVUTIL_SOCKET_MAX UINTPTR_MAX
#else
# define EVUTIL_SOCKET_MAX INT_MAX
#endif
#if !(LLONG_MAX >= EVUTIL_SOCKET_MAX)
# error "LLONG_MAX must be larger than EVUTIL_SOCKET_MAX"
#endif

/* To mark sockets as invalid */
#ifdef WIN32
# define EVUTIL_SOCKET_INVALID INVALID_SOCKET
#else
# define EVUTIL_SOCKET_INVALID -1
#endif

struct NeubotPoller {
#ifndef WIN32
	struct event evsignal;    /* for SIGINT */
#endif
	struct event_base *base;
	struct evdns_base *dnsbase;
};

struct NeubotEvent {
	neubot_hook_vo callback;
	neubot_hook_vo timeback;
	struct event ev;
	struct timeval tv;
	evutil_socket_t fileno;
	void *opaque;
};

struct ResolveContext {
	neubot_hook_vos callback;
	struct evbuffer *names;
	void *opaque;
};

int
neubot_socket_valid(long long socket)
{
	return (socket >= 0 && socket <= EVUTIL_SOCKET_MAX);
}

/*
 * NeubotEvent implementation
 */

static void
NeubotEvent_noop(void *opaque)
{
	/* nothing */ ;
}

static void
NeubotEvent_dispatch(evutil_socket_t socket, short event, void *opaque)
{
	struct NeubotEvent *nevp;

	nevp = (struct NeubotEvent *) opaque;
	if (event & EV_TIMEOUT)
		nevp->timeback(nevp->opaque);
	else
		nevp->callback(nevp->opaque);
	free(nevp);
}

static inline struct NeubotEvent *
NeubotEvent_construct(struct NeubotPoller *poller, long long fileno,
    neubot_hook_vo callback, neubot_hook_vo timeback, void *opaque,
    double timeout, short event)
{
	struct NeubotEvent *nevp;
	struct timeval *tvp;
	int result;

	nevp = NULL;

	/*
	 * Make sure that, if we want to do I/O, the socket is
	 * valid; otherwise, if we want to do timeout, make sure
	 * that the socket is invalid; while there, catch the
	 * case in which the user passes us an unexpected event.
	 */
	switch (event) {
	case EV_READ:
	case EV_WRITE:
		if (!neubot_socket_valid(fileno))
			goto cleanup;
		break;
	case EV_TIMEOUT:
		if (fileno != EVUTIL_SOCKET_INVALID)
			goto cleanup;
		break;
	default:
		goto cleanup;
	}

	if (callback == NULL)
		callback = NeubotEvent_noop;
	if (timeback == NULL)
		timeback = NeubotEvent_noop;

	nevp = calloc(1, sizeof (*nevp));
	if (nevp == NULL)
		goto cleanup;

	/*
	 * Note: `long long` simplifies the interaction with Java and
	 * shall be wide enough to hold evutil_socket_t, which is `int`
	 * on Unix and `uintptr_t` on Windows.
	 */
	nevp->fileno = (evutil_socket_t) fileno;
	nevp->callback = callback;
	nevp->timeback = timeback;
	nevp->opaque = opaque;

	event_set(&nevp->ev, nevp->fileno, event, NeubotEvent_dispatch, nevp);

	/* Treat negative timeouts as "infinite" */
	if (timeout >= 0.0) {
		neubot_timeval_init(&nevp->tv, timeout);
		tvp = &nevp->tv;
	} else
		tvp = NULL;

	result = event_add(&nevp->ev, tvp);
	if (result != 0)
		goto cleanup;

	return (nevp);

      cleanup:
	neubot_xfree(nevp);
	return (NULL);
}

/*
 * NeubotPoller implementation
 */

#ifndef WIN32
static void
NeubotPoller_sigint(int signo, short event, void *opaque)
{
	struct NeubotPoller *self;
	self = (struct NeubotPoller *) opaque;
	NeubotPoller_break_loop(self);
}
#endif

struct NeubotPoller *
NeubotPoller_construct(void)
{
	struct NeubotPoller *self;
	struct event_base *base;
	int retval;

	base = event_init();
	if (base == NULL)
		return (NULL);

	if (evdns_init() != 0)
		return (NULL);

	self = (struct NeubotPoller *) calloc(1, sizeof(*self));
	if (self == NULL)
		return (NULL);

	self->base = base;
	self->dnsbase = evdns_get_global_base();

#ifndef WIN32
	event_set(&self->evsignal, SIGINT, EV_SIGNAL,
	    NeubotPoller_sigint, self);

	retval = event_add(&self->evsignal, NULL);
	if (retval != 0)
		goto failure;
#endif

	return (self);

      failure:
#ifndef WIN32
	event_del(&self->evsignal);
#endif
	free(self);
	return (NULL);
}

/* Method that we use only internally: */
struct event_base *
NeubotPoller_event_base_(struct NeubotPoller *self)
{
	return (self->base);
}

/* Method that we use only internally: */
struct evdns_base *
NeubotPoller_evdns_base_(struct NeubotPoller *self)
{
	return (self->dnsbase);
}

int
NeubotPoller_sched(struct NeubotPoller *self, double delta,
    neubot_hook_vo callback, void *opaque)
{
	struct NeubotEvent *nevp;

	nevp = NeubotEvent_construct(self, EVUTIL_SOCKET_INVALID,
	    NeubotEvent_noop, callback, opaque, delta, EV_TIMEOUT);
	if (nevp == NULL)
		return (-1);
	return (0);
}

/*
 * The defer_read/defer_write/cancel interface allows to write
 * simpler code on the Python side, even though the interface is
 * less efficient than the Pollable interface.
 */

int
NeubotPoller_defer_read(struct NeubotPoller *self, long long fileno,
    neubot_hook_vo callback, neubot_hook_vo timeback, void *opaque,
    double timeout)
{
	struct NeubotEvent *nevp;

	nevp = NeubotEvent_construct(self, fileno, callback,
            timeback, opaque, timeout, EV_READ);
	if (nevp == NULL)
		return (-1);
	return (0);
}

int
NeubotPoller_defer_write(struct NeubotPoller *self, long long fileno,
    neubot_hook_vo callback, neubot_hook_vo timeback, void *opaque,
    double timeout)
{
	struct NeubotEvent *nevp;

	nevp = NeubotEvent_construct(self, fileno, callback,
            timeback, opaque, timeout, EV_WRITE);
	if (nevp == NULL)
		return (-1);
	return (0);
}

static void
NeubotPoller_resolve_callback_internal(int result, char type, int count,
    int ttl, void *addresses, void *opaque)
{
	struct ResolveContext *rc;
	const char *p;
	int error, family, size;
	char string[128];

	rc = (struct ResolveContext *) opaque;

	switch (type) {
	case DNS_IPv4_A:
		family = AF_INET;
		size = 4;
		break;
	case DNS_IPv6_AAAA:
		family = AF_INET6;
		size = 16;
		break;
	default:
		neubot_warn("resolve: invalid type");
		goto failure;
	}

	while (--count >= 0) {
		/* Note: address already in network byte order */
		p = inet_ntop(family, (char *)addresses + count * size,
		    string, sizeof (string));
		if (p == NULL) {
			neubot_warn("resolve: inet_ntop() failed");
			continue;
		}
		error = evbuffer_add(rc->names, p, strlen(p));
		if (error != 0) {
			neubot_warn("resolve: evbuffer_add() failed");
			goto failure;
		}
		error = evbuffer_add(rc->names, " ", 1);
		if (error != 0) {
			neubot_warn("resolve: evbuffer_add() failed");
			goto failure;
		}
	}

	error = evbuffer_add(rc->names, "\0", 1);
	if (error != 0) {
		neubot_warn("resolve: evbuffer_add() failed");
		goto failure;
	}
	p = (const char *) evbuffer_pullup(rc->names, -1);
	if (p == NULL) {
		neubot_warn("resolve: evbuffer_pullup() failed");
		goto failure;
	}

	rc->callback(rc->opaque, p);
	goto cleanup;

    failure:
	rc->callback(rc->opaque, "");
    cleanup:
	evbuffer_free(rc->names);
	free(rc);
}

int
NeubotPoller_resolve(struct NeubotPoller *poller, const char *family,
    const char *address, neubot_hook_vos callback, void *opaque)
{
	struct ResolveContext *rc;
	int result;

	rc = calloc(1, sizeof (*rc));
	if (rc == NULL) {
		neubot_warn("resolve: calloc() failed");
		goto failure;
	}

	rc->callback = callback;
	rc->opaque = opaque;

	rc->names = evbuffer_new();
	if (rc->names == NULL) {
		neubot_warn("resolve: evbuffer_new() failed");
		goto failure;
	}

	if (strcmp(family, "PF_INET6") == 0)
		result = evdns_resolve_ipv6(address, DNS_QUERY_NO_SEARCH,
		    NeubotPoller_resolve_callback_internal, rc);
	else if (strcmp(family, "PF_INET") == 0)
		result = evdns_resolve_ipv4(address, DNS_QUERY_NO_SEARCH,
		    NeubotPoller_resolve_callback_internal, rc);
	else {
		neubot_warn("resolve: invalid family");
		goto failure;
	}

	if (result != 0) {
		neubot_warn("resolve: evdns_resolve_ipvX() failed");
		goto failure;
	}

	return (0);

    failure:
	if (rc != NULL && rc->names != NULL)
		evbuffer_free(rc->names);
	if (rc != NULL)
		free(rc);

	return (-1);
}

void
NeubotPoller_loop(struct NeubotPoller *self)
{
	event_dispatch();
}

void
NeubotPoller_break_loop(struct NeubotPoller *self)
{
	event_loopbreak();
}
