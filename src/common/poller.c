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

#include <event2/buffer.h>
#include <event2/event.h>
#include <event2/event_compat.h>
#include <event2/event_struct.h>

#include <event2/dns.h>
#include <event2/dns_compat.h>

#include "ight_wrappers.h"

#include "net/ll2sock.h"

#include "common/poller.h"
#include "common/utils.h"
#include "common/log.h"

struct IghtPoller {
#ifndef WIN32
	struct event evsignal;    /* for SIGINT */
#endif
	struct event_base *base;
	struct evdns_base *dnsbase;
};

struct IghtEvent {
	ight_hook_vo callback;
	ight_hook_vo timeback;
	struct event ev;
	struct timeval tv;
	evutil_socket_t fileno;
	void *opaque;
};

struct ResolveContext {
	ight_hook_vos callback;
	struct evbuffer *names;
	void *opaque;
};

/*
 * IghtEvent implementation
 */

static void
IghtEvent_noop(void *opaque)
{
	(void) opaque;

	/* nothing */ ;
}

static void
IghtEvent_dispatch(evutil_socket_t socket, short event, void *opaque)
{
	struct IghtEvent *nevp;

	(void) socket;

	nevp = (struct IghtEvent *) opaque;
	if (event & EV_TIMEOUT)
		nevp->timeback(nevp->opaque);
	else
		nevp->callback(nevp->opaque);
	free(nevp);
}

static inline struct IghtEvent *
IghtEvent_construct(struct IghtPoller *poller, long long fileno,
    ight_hook_vo callback, ight_hook_vo timeback, void *opaque,
    double timeout, short event)
{
	struct IghtEvent *nevp;
	struct timeval *tvp;
	int result;

	(void) poller;

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
		if (!ight_socket_valid(fileno))
			goto cleanup;
		break;
	case EV_TIMEOUT:
		if (fileno != IGHT_SOCKET_INVALID)
			goto cleanup;
		break;
	default:
		abort();
	}

	if (callback == NULL)
		callback = IghtEvent_noop;
	if (timeback == NULL)
		timeback = IghtEvent_noop;

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

	event_set(&nevp->ev, nevp->fileno, event, IghtEvent_dispatch, nevp);

	tvp = ight_timeval_init(&nevp->tv, timeout);

	result = event_add(&nevp->ev, tvp);
	if (result != 0)
		goto cleanup;

	return (nevp);

      cleanup:
	ight_xfree(nevp);
	return (NULL);
}

/*
 * IghtPoller implementation
 */

#ifndef WIN32
static void
IghtPoller_sigint(int signo, short event, void *opaque)
{
	struct IghtPoller *self;

	(void) signo;
	(void) event;

	self = (struct IghtPoller *) opaque;
	IghtPoller_break_loop(self);
}
#endif

struct IghtPoller *
IghtPoller_construct(void)
{
	struct IghtPoller *self;
	struct event_base *base;
	int retval;

	base = event_init();
	if (base == NULL)
		return (NULL);

	if (evdns_init() != 0)
		return (NULL);

	self = (struct IghtPoller *) calloc(1, sizeof(*self));
	if (self == NULL)
		return (NULL);

	self->base = base;
	self->dnsbase = evdns_get_global_base();
	if (self->dnsbase == NULL)
		abort();

#ifndef WIN32
	event_set(&self->evsignal, SIGINT, EV_SIGNAL,
	    IghtPoller_sigint, self);

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
IghtPoller_event_base_(struct IghtPoller *self)
{
	return (self->base);
}

/* Method that we use only internally: */
struct evdns_base *
IghtPoller_evdns_base_(struct IghtPoller *self)
{
	return (self->dnsbase);
}

/*
 * This is implemented like in Neubot; however, it is a bit dangerous
 * and/or annoying that one cannot destroy pending callbacks.
 */
int
IghtPoller_sched(struct IghtPoller *self, double delta,
    ight_hook_vo callback, void *opaque)
{
	struct IghtEvent *nevp;

	nevp = IghtEvent_construct(self, IGHT_SOCKET_INVALID,
	    IghtEvent_noop, callback, opaque, delta, EV_TIMEOUT);
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
IghtPoller_defer_read(struct IghtPoller *self, long long fileno,
    ight_hook_vo callback, ight_hook_vo timeback, void *opaque,
    double timeout)
{
	struct IghtEvent *nevp;

	nevp = IghtEvent_construct(self, fileno, callback,
            timeback, opaque, timeout, EV_READ);
	if (nevp == NULL)
		return (-1);
	return (0);
}

int
IghtPoller_defer_write(struct IghtPoller *self, long long fileno,
    ight_hook_vo callback, ight_hook_vo timeback, void *opaque,
    double timeout)
{
	struct IghtEvent *nevp;

	nevp = IghtEvent_construct(self, fileno, callback,
            timeback, opaque, timeout, EV_WRITE);
	if (nevp == NULL)
		return (-1);
	return (0);
}

static void
IghtPoller_resolve_callback_internal(int result, char type, int count,
    int ttl, void *addresses, void *opaque)
{
	struct ResolveContext *rc;
	const char *p;
	int error, family, size;
	char string[128];

	(void) result;
	(void) ttl;

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
		abort();
	}

	while (--count >= 0) {
		/* Note: address already in network byte order */
		p = inet_ntop(family, (char *)addresses + count * size,
		    string, sizeof (string));
		if (p == NULL) {
			ight_warn("resolve: inet_ntop() failed");
			continue;
		}
		error = evbuffer_add(rc->names, p, strlen(p));
		if (error != 0) {
			ight_warn("resolve: evbuffer_add() failed");
			goto failure;
		}
		error = evbuffer_add(rc->names, " ", 1);
		if (error != 0) {
			ight_warn("resolve: evbuffer_add() failed");
			goto failure;
		}
	}

	error = evbuffer_add(rc->names, "\0", 1);
	if (error != 0) {
		ight_warn("resolve: evbuffer_add() failed");
		goto failure;
	}
	p = (const char *) evbuffer_pullup(rc->names, -1);
	if (p == NULL) {
		ight_warn("resolve: evbuffer_pullup() failed");
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
IghtPoller_resolve(struct IghtPoller *poller, const char *family,
    const char *address, ight_hook_vos callback, void *opaque)
{
	struct ResolveContext *rc;
	int result;

	(void) poller;

	rc = calloc(1, sizeof (*rc));
	if (rc == NULL) {
		ight_warn("resolve: calloc() failed");
		goto failure;
	}

	rc->callback = callback;
	rc->opaque = opaque;

	rc->names = evbuffer_new();
	if (rc->names == NULL) {
		ight_warn("resolve: evbuffer_new() failed");
		goto failure;
	}

	if (strcmp(family, "PF_INET6") == 0)
		result = evdns_resolve_ipv6(address, DNS_QUERY_NO_SEARCH,
		    IghtPoller_resolve_callback_internal, rc);
	else if (strcmp(family, "PF_INET") == 0)
		result = evdns_resolve_ipv4(address, DNS_QUERY_NO_SEARCH,
		    IghtPoller_resolve_callback_internal, rc);
	else {
		ight_warn("resolve: invalid family");
		goto failure;
	}

	if (result != 0) {
		ight_warn("resolve: evdns_resolve_ipvX() failed");
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
IghtPoller_loop(struct IghtPoller *self)
{
	(void) self;

	event_dispatch();
}

void
IghtPoller_break_loop(struct IghtPoller *self)
{
	(void) self;

	event_loopbreak();
}
