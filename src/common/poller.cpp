/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#include <limits.h>
#include <string.h>
#include <stdexcept>
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

/*
 * IghtDelayedCall implementation
 */

IghtDelayedCall::IghtDelayedCall(double t, std::function<void(void)> &&f)
{
	timeval timeo;

	this->func = new std::function<void(void)>();

	if ((this->evp = event_new(ight_get_global_event_base(),
	    IGHT_SOCKET_INVALID, EV_TIMEOUT, this->dispatch,
	    this->func)) == NULL) {
		delete (this->func);
		throw std::bad_alloc();
	}

	if (event_add(this->evp, ight_timeval_init(&timeo, t)) != 0) {
		delete (this->func);
		event_free(this->evp);
		throw std::runtime_error("cannot register new event");
	}

	std::swap(*this->func, f);
}

void
IghtDelayedCall::dispatch(evutil_socket_t socket, short event, void *opaque)
{
	auto funcptr = static_cast<std::function<void(void)> *>(opaque);
	if (*funcptr)
		(*funcptr)();

	// Avoid compiler warning
	(void) socket;
	(void) event;
}

IghtDelayedCall::~IghtDelayedCall(void)
{
	delete (this->func);  /* delete handles NULL */
	if (this->evp)
		event_free(this->evp);
}

/*
 * IghtPoller implementation
 */

#ifndef WIN32
static void
IghtPoller_sigint(int signo, short event, void *opaque)
{
	(void) signo;
	(void) event;

	auto self = (IghtPoller *) opaque;
	self->break_loop();
}
#endif

/*
 * TODO: Here we should use the new libevent API, which does not
 * use global event_base and evdns_base.
 */
IghtPoller::IghtPoller(void)
{
	if ((this->base = event_init()) == NULL)
		throw std::bad_alloc();

	if (evdns_init() != 0)
		throw std::bad_alloc();

	if ((this->dnsbase = evdns_get_global_base()) == NULL)
		throw std::runtime_error("unexpected libevent error");

#ifndef WIN32
	if ((this->evsignal = event_new(this->base, SIGINT, EV_SIGNAL,
	    IghtPoller_sigint, this)) == NULL) {
		throw std::bad_alloc();
	}

	if (event_add(this->evsignal, NULL) != 0) {
		event_free(this->evsignal);
		throw std::runtime_error("unexpected libevent error");
	}
#endif
}

IghtPoller::~IghtPoller(void)
{
	event_free(this->evsignal);
}

void
IghtPoller::loop(void)
{
	event_dispatch();
}

void
IghtPoller::break_loop(void)
{
	event_loopbreak();
}

/*
 * API to access/use global objects.
 */

IghtPoller GLOBAL_POLLER;

IghtPoller *
ight_get_global_poller(void)
{
	return (&GLOBAL_POLLER);
}

event_base *
ight_get_global_event_base(void)
{
	return (GLOBAL_POLLER.get_event_base());
}

evdns_base *
ight_get_global_evdns_base(void)
{
	return (GLOBAL_POLLER.get_evdns_base());
}

void
ight_loop(void)
{
	GLOBAL_POLLER.loop();
}

void
ight_break_loop(void)
{
	GLOBAL_POLLER.break_loop();
}
