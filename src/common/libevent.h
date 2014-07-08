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
};

extern IghtLibevent IGHT_LIBEVENT;	/* Default libevent wrapper */

#endif  // LIBIGHT_COMMON_LIBEVENT_H
