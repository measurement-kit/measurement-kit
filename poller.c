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

#include <sys/queue.h>

#include <string.h>
#include <stdlib.h>
#include <math.h>

#ifndef WIN32
# include <signal.h>
#endif

#include <event.h>

#include "log.h"
#include "neubot.h"
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

struct NeubotPollable {
	TAILQ_ENTRY(NeubotPollable) entry;
	NeubotPollable_callback handle_close;
	NeubotPollable_callback handle_read;
	NeubotPollable_callback handle_write;
	evutil_socket_t fileno;
	struct NeubotPoller *poller;
	double timeout;
	struct event evread;
	struct event evwrite;
	void *opaque;
	unsigned isattached;
};

struct NeubotPoller {
	TAILQ_HEAD(, NeubotPollable) head;
#ifndef WIN32
	struct event evsignal;    /* for SIGINT */
#endif
};

struct NeubotEvent {
	NeubotPoller_callback callback;
	NeubotPoller_callback timeback;
	struct event ev;
	struct timeval tv;
	evutil_socket_t fileno;
	void *opaque;
};

static inline int
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
	NeubotPoller_callback callback;
	void *nevp_opaque;

	nevp = (struct NeubotEvent *) opaque;
	if (event & EV_TIMEOUT)
		callback = nevp->timeback;
	else
		callback = nevp->callback;
	nevp_opaque = nevp->opaque;
	free(nevp);
	callback(nevp_opaque);
}

static inline struct NeubotEvent *
NeubotEvent_construct(struct NeubotPoller *poller, long long fileno,
    NeubotPoller_callback callback, NeubotPoller_callback timeback,
    void *opaque, double timeout, short event)
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
		if (fileno != -1)
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

void
NeubotEvent_cancel(struct NeubotEvent *nevp)
{
	event_del(&nevp->ev);
	free(nevp);
}

/*
 * NeubotPoller implementation
 */

static inline void
NeubotPoller_register_pollable(struct NeubotPoller *self,
    struct NeubotPollable *pollable)
{
	TAILQ_INSERT_TAIL(&self->head, pollable, entry);
}

static inline void
NeubotPoller_unregister_pollable(struct NeubotPoller *self,
    struct NeubotPollable *pollable)
{
	TAILQ_REMOVE(&self->head, pollable, entry);
}

static void
NeubotPoller_periodic(void *opaque)
{
	struct NeubotPoller *self;
	struct NeubotPollable *pollable;
	struct NeubotPollable *tmp;
	double curtime;
	int retval;

	self = (struct NeubotPoller *) opaque;
	retval = NeubotPoller_sched(self, 10.0, NeubotPoller_periodic, self);
	if (retval != 0)
		abort();	/* XXX */

	curtime = neubot_time_now();

	pollable = TAILQ_FIRST(&self->head);
	while (pollable != NULL) {
		if (pollable->timeout >= 0.0 && curtime > pollable->timeout) {
			neubot_warn("poller.c: watchdog timeout");
			tmp = pollable;
			pollable = TAILQ_NEXT(pollable, entry);
			NeubotPollable_close(tmp);
		} else
			pollable = TAILQ_NEXT(pollable, entry);
	}
}

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

	self = (struct NeubotPoller *) calloc(1, sizeof(*self));
	if (self == NULL)
		goto failure;

	TAILQ_INIT(&self->head);

	base = event_init();
	if (base == NULL)
		goto failure;

#ifndef WIN32
	event_set(&self->evsignal, SIGINT, EV_SIGNAL,
	    NeubotPoller_sigint, self);
	retval = event_add(&self->evsignal, NULL);
	if (retval != 0)
		goto failure;
#endif

	retval = NeubotPoller_sched(self, 10.0, NeubotPoller_periodic, self);
	if (retval != 0)
		goto failure;

	return (self);

      failure:
	/* TODO: make sure we close everything */
	if (self != NULL)
		free(self);
	return (NULL);
}

int
NeubotPoller_sched(struct NeubotPoller *self, double delta,
    NeubotPoller_callback callback, void *opaque)
{
	struct NeubotEvent *nevp;

	nevp = NeubotEvent_construct(self, -1, NeubotEvent_noop,
	    callback, opaque, delta, EV_TIMEOUT);
	if (nevp == NULL)
		return (-1);
	return (0);
}

/*
 * The defer_read/defer_write/cancel interface allows to write
 * simpler code on the Python side, even though the interface is
 * less efficient than the Pollable interface.
 */

struct NeubotEvent *
NeubotPoller_defer_read(struct NeubotPoller *self, long long fileno,
    NeubotPoller_callback callback, NeubotPoller_callback timeback,
    void *opaque, double timeout)
{
	return (NeubotEvent_construct(self, fileno, callback,
            timeback, opaque, timeout, EV_READ));
}

struct NeubotEvent *
NeubotPoller_defer_write(struct NeubotPoller *self, long long fileno,
    NeubotPoller_callback callback, NeubotPoller_callback timeback,
    void *opaque, double timeout)
{
	return (NeubotEvent_construct(self, fileno, callback,
            timeback, opaque, timeout, EV_WRITE));
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

/*
 * NeubotPollable implementation
 */

static void
NeubotPollable_noop(struct NeubotPollable *self)
{
	/* nothing */ ;
}

static void
NeubotPollable_dispatch(evutil_socket_t filenum, short event, void *opaque)
{
	struct NeubotPollable *pollable;

	pollable = (struct NeubotPollable *) opaque;

	if (event & EV_READ)
		pollable->handle_read(pollable);
	else if (event & EV_WRITE)
		pollable->handle_write(pollable);
	else
		/* nothing */ ;
}

struct NeubotPollable *
NeubotPollable_construct(struct NeubotPoller *poller, NeubotPollable_callback
    handle_read, NeubotPollable_callback handle_write, NeubotPollable_callback
    handle_close, void *opaque)
{
	struct NeubotPollable *self;

	if (handle_read == NULL)
		handle_read = NeubotPollable_noop;
	if (handle_write == NULL)
		handle_write = NeubotPollable_noop;
	if (handle_close == NULL)
		handle_close = NeubotPollable_noop;

	self = calloc(1, sizeof(*self));
	if (self == NULL)
		return (NULL);

	self->poller = poller;
	self->handle_close = handle_close;
	self->handle_read = handle_read;
	self->handle_write = handle_write;
	self->fileno = EVUTIL_SOCKET_INVALID;
	self->opaque = opaque;
	self->timeout = -1.0;

	return (self);
}

void *
NeubotPollable_opaque(struct NeubotPollable *self)
{
	return (self->opaque);
}

struct NeubotPoller *
NeubotPollable_poller(struct NeubotPollable *self)
{
	return (self->poller);
}

/*
 * Note: attach() is separated from construct(), because in Neubot there
 * are cases in which we create a pollable and then we attach() and detach()
 * file descriptors to it (e.g., the Connector object does that).
 */

int
NeubotPollable_attach(struct NeubotPollable *self, long long fileno)
{
	if (self->isattached)
		return (-1);
	if (!neubot_socket_valid(fileno))
		return (-1);
	/*
	 * Note: `long long` simplifies the interaction with Java and
	 * shall be wide enough to hold evutil_socket_t, which is `int`
	 * on Unix and `uintptr_t` on Windows.
	 */
	self->fileno = (evutil_socket_t) fileno;
	event_set(&self->evread, self->fileno, EV_READ | EV_PERSIST,
	    NeubotPollable_dispatch, self);
	event_set(&self->evwrite, self->fileno, EV_WRITE | EV_PERSIST,
	    NeubotPollable_dispatch, self);
	NeubotPoller_register_pollable(self->poller, self);
	self->isattached = 1;
	return (0);
}

void
NeubotPollable_detach(struct NeubotPollable *self)
{
	if (self->isattached) {
		self->fileno = EVUTIL_SOCKET_INVALID;
		event_del(&self->evread);
		event_del(&self->evwrite);
		NeubotPoller_unregister_pollable(self->poller, self);
		self->isattached = 0;
	}
}

long long
NeubotPollable_fileno(struct NeubotPollable *self)
{
	if (self->isattached)
		return ((long long) self->fileno);
	else
		return (-1);
}

int
NeubotPollable_set_readable(struct NeubotPollable *self)
{
	if (!self->isattached)
		return (-1);
	return (event_add(&self->evread, NULL));
}

int
NeubotPollable_unset_readable(struct NeubotPollable *self)
{
	if (!self->isattached)
		return (-1);
	return (event_del(&self->evread));
}

int
NeubotPollable_set_writable(struct NeubotPollable *self)
{
	if (!self->isattached)
		return (-1);
	return (event_add(&self->evwrite, NULL));
}

int
NeubotPollable_unset_writable(struct NeubotPollable *self)
{
	if (!self->isattached)
		return (-1);
	return (event_del(&self->evwrite));
}

void
NeubotPollable_set_timeout(struct NeubotPollable *self, double timeout)
{
	self->timeout = neubot_time_now() + timeout;
}

void
NeubotPollable_clear_timeout(struct NeubotPollable *self)
{
	self->timeout = -1.0;
}

void
NeubotPollable_close(struct NeubotPollable *self)
{
	NeubotPollable_detach(self);
	self->handle_close(self);
	free(self);
}
