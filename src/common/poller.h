/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#ifndef LIBIGHT_POLLER_H
# define LIBIGHT_POLLER_H

#include <event2/util.h>	/* for evutil_socket_t */
#include <stddef.h>		/* for NULL */

#include <functional>

struct event_base;
struct evdns_base;
struct event;

class IghtDelayedCall {

	/*
	 * The function must be a pointer and cannot be an object, because
	 * we need to pass a stable pointer to event_new().
	 */
	std::function<void(void)> *func = NULL;
	event *evp = NULL;

	// Callback for libevent
	static void dispatch(evutil_socket_t, short, void *);

    public:
	IghtDelayedCall(void) {
		/* nothing */
	}

	IghtDelayedCall(double, std::function<void(void)>&&);
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
		std::swap(this->evp, d.evp);
		std::swap(this->func, d.func);
	}
	IghtDelayedCall& operator=(IghtDelayedCall&& d) {
		std::swap(this->evp, d.evp);
		std::swap(this->func, d.func);
		return (*this);
	}
};

class IghtPoller {

	event_base *base;
	evdns_base *dnsbase;
	event *evsignal;		/* for SIGINT on UNIX */

    public:
	IghtPoller(void);
	~IghtPoller(void);

	event_base *get_event_base(void) {
		return (this->base);
	}

	evdns_base *get_evdns_base(void) {
		return (this->dnsbase);
	}

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
