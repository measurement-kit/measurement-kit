/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#ifndef LIBIGHT_POLLER_H
# define LIBIGHT_POLLER_H

#include "common/constraints.hpp"
#include "common/libevent.h"

#include <functional>

class IghtDelayedCall : public ight::common::constraints::NonCopyable,
		public ight::common::constraints::NonMovable {

	/*
	 * A previous implementation of this class required `func` to
	 * be a pointer. The current implementation does not. So we can
	 * rewrite the code to use an object rather than a pointer.
	 */
	std::function<void(void)> *func = NULL;
	event *evp = NULL;
	IghtLibevent *libevent = IghtGlobalLibevent::get();

	// Callback for libevent
	static void dispatch(evutil_socket_t, short, void *);

    public:
	IghtDelayedCall(double, std::function<void(void)>&&,
	    IghtLibevent *libevent = NULL, event_base *evbase = NULL);
	~IghtDelayedCall(void);
};

class IghtPoller : public ight::common::constraints::NonCopyable,
		public ight::common::constraints::NonMovable {

	event_base *base;
	evdns_base *dnsbase;
	event *evsignal;		/* for SIGINT on UNIX */

	IghtLibevent *libevent = IghtGlobalLibevent::get();

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
};

struct IghtGlobalPoller {
	static IghtPoller *get(void) {
		static IghtPoller singleton;
		return (&singleton);
	}
};

/*
 * Syntactic sugar:
 */

inline IghtPoller *ight_get_global_poller(void) {
	return (IghtGlobalPoller::get());
}

inline event_base *ight_get_global_event_base(void) {
	return (IghtGlobalPoller::get()->get_event_base());
}

inline evdns_base *ight_get_global_evdns_base(void) {
	return (IghtGlobalPoller::get()->get_evdns_base());
}

inline void ight_loop(void) {
	IghtGlobalPoller::get()->loop();
}

inline void ight_break_loop(void) {
	IghtGlobalPoller::get()->break_loop();
}

#endif  // LIBIGHT_POLLER_H
