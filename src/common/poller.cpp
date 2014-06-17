/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#include <limits.h>
#include <string.h>
#include <stdlib.h>

#ifndef WIN32
# include <signal.h>
#endif

#include <event2/event.h>
#include <event2/event_compat.h>
#include <event2/event_struct.h>

#include <event2/dns.h>
#include <event2/dns_compat.h>

#include "net/ll2sock.h"

#include "common/poller.h"
#include "common/utils.h"
#include "common/log.h"

struct IghtPoller {
#ifndef WIN32
	struct event *evsignal;    /* for SIGINT */
#endif
	struct event_base *base;
	struct evdns_base *dnsbase;
};

struct IghtEvent {
	ight_hook_vo timeback;
	struct event ev;
	struct timeval tv;
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
	(void) socket;
	(void) event;

	auto nevp = (struct IghtEvent *) opaque;
	nevp->timeback(nevp->opaque);
	free(nevp);
}

static inline struct IghtEvent *
IghtEvent_construct(struct IghtPoller *poller, ight_hook_vo timeback,
    void *opaque, double timeout)
{
	struct IghtEvent *nevp;
	struct timeval *tvp;
	int result;

	(void) poller;

	nevp = NULL;

	if (timeback == NULL)
		timeback = IghtEvent_noop;

	nevp = (struct IghtEvent *) calloc(1, sizeof (*nevp));
	if (nevp == NULL)
		goto cleanup;

	nevp->timeback = timeback;
	nevp->opaque = opaque;

	event_set(&nevp->ev, IGHT_SOCKET_INVALID, EV_TIMEOUT,
	    IghtEvent_dispatch, nevp);

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
	self->evsignal = event_new(self->base, SIGINT, EV_SIGNAL,
	    IghtPoller_sigint, self);
	if (self->evsignal == NULL)
		goto failure;

	retval = event_add(self->evsignal, NULL);
	if (retval != 0)
		goto failure;
#endif

	return (self);

      failure:
#ifndef WIN32
	if (self->evsignal)
		event_free(self->evsignal);
#endif
	free(self);
	return (NULL);
}

struct event_base *
IghtPoller_get_event_base(struct IghtPoller *self)
{
	return (self->base);
}

struct evdns_base *
IghtPoller_get_evdns_base(struct IghtPoller *self)
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

	nevp = IghtEvent_construct(self, callback, opaque, delta);
	if (nevp == NULL)
		return (-1);
	return (0);
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
