/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

//
// Libevent abstraction layer.
//

#ifndef LIBIGHT_COMMON_LIBEVENT_H
# define LIBIGHT_COMMON_LIBEVENT_H

#include <event2/buffer.h>
#include <event2/dns.h>
#include <event2/event.h>

#include <functional>

struct IghtLibevent {

	/*
	 * event_base
	 */

	std::function<event_base*(void)> event_base_new =
	    ::event_base_new;

	std::function<int(event_base*)> event_base_dispatch =
	    ::event_base_dispatch;

	std::function<int(event_base*)> event_base_loopbreak =
	    ::event_base_loopbreak;

	std::function<void(event_base*)> event_base_free =
	    ::event_base_free;

	/*
	 * evdns_base
	 */

	std::function<evdns_base*(event_base*, int)> evdns_base_new =
	    ::evdns_base_new;

	std::function<void(evdns_base*, int)> evdns_base_free =
	    ::evdns_base_free;

	/*
	 * event
	 */

	std::function<event*(event_base*, evutil_socket_t, short,
	    event_callback_fn, void*)> event_new = ::event_new;

	std::function<int(event*, timeval*)> event_add = ::event_add;

	std::function<int(event*)> event_del = ::event_del;

	std::function<void(event*)> event_free = ::event_free;

	/*
	 * evbuffer
	 */

	std::function<evbuffer*(void)> evbuffer_new = ::evbuffer_new;

	std::function<void(evbuffer*)> evbuffer_free = ::evbuffer_free;
};

struct IghtGlobalLibevent {

	IghtGlobalLibevent(void) {
		/* nothing */
	}

	static IghtLibevent *get(void) {
		static IghtLibevent singleton;
		return (&singleton);
	}

	IghtGlobalLibevent(IghtGlobalLibevent&) = delete;
	IghtGlobalLibevent& operator=(IghtGlobalLibevent&) = delete;
	IghtGlobalLibevent(IghtGlobalLibevent&&) = delete;
	IghtGlobalLibevent& operator=(IghtGlobalLibevent&&) = delete;
};

class IghtEvbuffer {
	IghtLibevent *libevent = IghtGlobalLibevent::get();
	evbuffer *evbuf = NULL;

    public:
	IghtEvbuffer(IghtLibevent *lev = NULL) {
		if (lev != NULL)
			libevent = lev;
	}

	~IghtEvbuffer(void) {
		if (evbuf != NULL)
			libevent->evbuffer_free(evbuf);
	}

	operator evbuffer *(void) {
		if (evbuf == NULL &&
		    (evbuf = libevent->evbuffer_new()) == NULL)
			throw std::bad_alloc();
		return (evbuf);
	}

	IghtLibevent *get_libevent(void) {
		return (libevent);
	}

	IghtEvbuffer(IghtEvbuffer&) = delete;
	IghtEvbuffer& operator=(IghtEvbuffer&) = delete;

	IghtEvbuffer(IghtEvbuffer&& other) {
		std::swap(evbuf, other.evbuf);
		std::swap(libevent, other.libevent);
	}
	IghtEvbuffer& operator=(IghtEvbuffer&& other) {
		std::swap(evbuf, other.evbuf);
		std::swap(libevent, other.libevent);
		return (*this);
	}
};

#endif  // LIBIGHT_COMMON_LIBEVENT_H
