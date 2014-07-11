/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#ifndef LIBIGHT_POLLER_H
# define LIBIGHT_POLLER_H

#include "common/libevent.h"

#include <functional>

class IghtDelayedCall {

	/*
	 * The function must be a pointer and cannot be an object, because
	 * we need to pass a stable pointer to event_new().
	 */
	std::function<void(void)> *func = NULL;
	event *evp = NULL;
	IghtLibevent *libevent = &IGHT_LIBEVENT;

	// Callback for libevent
	static void dispatch(evutil_socket_t, short, void *);

    public:
	IghtDelayedCall(void) {
		/* nothing */
	}

	IghtDelayedCall(double, std::function<void(void)>&&,
	    IghtLibevent *libevent = NULL, event_base *evbase = NULL);
	~IghtDelayedCall(void);

	/*
	 * It does not have sense to make a copy of this class, since we
	 * don't want to manage/refcount multiple copies of `evp`.
	 */
	IghtDelayedCall(const IghtDelayedCall&) = delete;
	IghtDelayedCall& operator=(const IghtDelayedCall& other) = delete;

	/*
	 * Enable move semantic.
	 */
	IghtDelayedCall(IghtDelayedCall&& d) {
		std::swap(this->libevent, d.libevent);
		std::swap(this->evp, d.evp);
		std::swap(this->func, d.func);
	}
	IghtDelayedCall& operator=(IghtDelayedCall&& d) {
		std::swap(this->libevent, d.libevent);
		std::swap(this->evp, d.evp);
		std::swap(this->func, d.func);
		return (*this);
	}
};

class IghtPoller {

	event_base *base;
	evdns_base *dnsbase;
	event *evsignal;		/* for SIGINT on UNIX */

	IghtLibevent *libevent = &IGHT_LIBEVENT;

    public:
	IghtPoller(IghtLibevent *libevent = NULL);
	~IghtPoller(void);

	event_base *get_event_base(void) {
		return (this->base);
	}

	evdns_base *get_evdns_base(void) {
		return (this->dnsbase);
	}

	/*
	 * Register a SIGINT handler that breaks the poller loop when
	 * the SIGINT signal is received. Generally speaking, a library
	 * should provide mechanism, not policy. However, this method
	 * is meant to be used in test programs only.
	 *
	 * The use case for which this functionality exists, in particular,
	 * is the following: I want ^C to break the poller loop and lead
	 * to a clean exit, so Valgrind can check whether there are leaks
	 * for long running test programs (i.e., servers).
	 */
	void break_loop_on_sigint_(int enable = 1);

	void loop(void);

	void break_loop(void);

	/*
	 * No copy and no move.
	 */
	IghtPoller(const IghtPoller&) = delete;
	IghtPoller& operator=(const IghtPoller& other) = delete;
	IghtPoller(const IghtPoller&&) = delete;
	IghtPoller& operator=(const IghtPoller&& other) = delete;
};

IghtPoller *ight_get_global_poller(void);

event_base *ight_get_global_event_base(void);

evdns_base *ight_get_global_evdns_base(void);

void ight_loop(void);

void ight_break_loop(void);

#endif  // LIBIGHT_POLLER_H
