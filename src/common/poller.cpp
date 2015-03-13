/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#include <string.h>
#include <stdexcept>
#include <stdlib.h>

#ifndef WIN32
# include <signal.h>
#endif

#include <ight/common/poller.hpp>
#include <ight/common/utils.hpp>
#include <ight/common/log.hpp>

using namespace ight::common::libevent;
using namespace ight::common::poller;

/*
 * DelayedCall implementation
 */

DelayedCall::DelayedCall(double t, std::function<void(void)> &&f,
    Libevent *libevent, event_base *evbase)
{
	timeval timeo;

	if (libevent != NULL)
		this->libevent = libevent;
	if (evbase == NULL)
		evbase = ight_get_global_event_base();

	this->func = new std::function<void(void)>();

	if ((this->evp = this->libevent->event_new(evbase, IGHT_SOCKET_INVALID,
	    EV_TIMEOUT, this->dispatch, this->func)) == NULL) {
		delete (this->func);
		throw std::bad_alloc();
	}

	if (this->libevent->event_add(this->evp, ight_timeval_init(
	    &timeo, t)) != 0) {
		delete (this->func);
		this->libevent->event_free(this->evp);
		throw std::runtime_error("cannot register new event");
	}

	std::swap(*this->func, f);
}

void
DelayedCall::dispatch(evutil_socket_t socket, short event, void *opaque)
{
	auto funcptr = static_cast<std::function<void(void)> *>(opaque);
	if (*funcptr)
		(*funcptr)();

	// Avoid compiler warning
	(void) socket;
	(void) event;
}

DelayedCall::~DelayedCall(void)
{
	delete (this->func);  /* delete handles NULL */
	if (this->evp)
		this->libevent->event_free(this->evp);
}

/*
 * Poller implementation
 */

#ifndef WIN32
static void
Poller_sigint(int signo, short event, void *opaque)
{
	(void) signo;
	(void) event;

	auto self = (Poller *) opaque;
	self->break_loop();
}
#endif

Poller::Poller(Libevent *libevent)
{
	if (libevent != NULL)
		this->libevent = libevent;

	if ((this->base = this->libevent->event_base_new()) == NULL) {
		throw std::bad_alloc();
	}

	if ((this->dnsbase = this->libevent->evdns_base_new(
	    this->base, 1)) == NULL) {
		this->libevent->event_base_free(this->base);
		throw std::bad_alloc();
	}

#ifndef WIN32
	/*
	 * Note: The move semantic is incompatible with this object
	 * because we pass `this` to `event_new()`.
	 */
	if ((this->evsignal = this->libevent->event_new(this->base, SIGINT,
	    EV_SIGNAL, Poller_sigint, this)) == NULL) {
		this->libevent->evdns_base_free(this->dnsbase, 1);
		this->libevent->event_base_free(this->base);
		throw std::bad_alloc();
	}
#endif
}

Poller::~Poller(void)
{
	this->libevent->event_free(this->evsignal);
	this->libevent->evdns_base_free(this->dnsbase, 1);
	this->libevent->event_base_free(this->base);
}

void
Poller::break_loop_on_sigint_(int enable)
{
#ifndef WIN32
	/*
	 * XXX There is a comment in libevent/signal.c saying that, for
	 * historical reasons, "only one event base can be set up to use
	 * [signals] at a time", unless kqueue is used as backend.
	 *
	 * We don't plan to use multiple pollers and this function is not
	 * meant to be used by the real Android application; yet, it is
	 * good to remember that this limitation exists.
	 */
	if (enable) {
		if (this->libevent->event_add(this->evsignal, NULL) != 0)
			throw std::runtime_error("cannot add SIGINT event");
	} else {
		if (this->libevent->event_del(this->evsignal) != 0)
			throw std::runtime_error("cannot del SIGINT event");
	}
#endif
}

void
Poller::loop(void)
{
	auto result = this->libevent->event_base_dispatch(this->base);
	if (result < 0)
		throw std::runtime_error("event_base_dispatch() failed");
	if (result == 1)
		ight_warn("loop: no pending and/or active events");
}

void Poller::loop_once(void) {
    auto result = this->libevent->event_base_loop(this->base, EVLOOP_ONCE);
    if (result < 0)
        throw std::runtime_error("event_base_loop() failed");
    if (result == 1)
        ight_warn("loop: no pending and/or active events");
}

void
Poller::break_loop(void)
{
	if (this->libevent->event_base_loopbreak(this->base) != 0)
		throw std::runtime_error("event_base_loopbreak() failed");
}
