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
#include <event2/bufferevent.h>
#include <event2/dns.h>
#include <event2/event.h>

#include <arpa/inet.h>

#include <functional>
#include <stdexcept>

#include "common/constraints.hpp"

struct IghtLibevent {

	/*
	 * bufferevent
	 */

	std::function<bufferevent *(event_base *, evutil_socket_t, int)>
	    bufferevent_socket_new = ::bufferevent_socket_new;

	std::function<void(bufferevent *)>
	    bufferevent_free = ::bufferevent_free;

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

	std::function<int(evdns_base *, const char *)>
	    evdns_base_nameserver_ip_add = ::evdns_base_nameserver_ip_add;

	std::function<int(evdns_base *, const char *, const char *)>
	    evdns_base_set_option = ::evdns_base_set_option;

	std::function<evdns_request *(evdns_base *, const char *, int,
	    evdns_callback_type, void *)> evdns_base_resolve_ipv4 =
	    ::evdns_base_resolve_ipv4;

	std::function<evdns_request *(evdns_base *, const char *, int,
	    evdns_callback_type, void *)> evdns_base_resolve_ipv6 =
	    ::evdns_base_resolve_ipv6;

	std::function<evdns_request *(evdns_base *, const struct in_addr *, int,
	    evdns_callback_type, void *)> evdns_base_resolve_reverse =
	    ::evdns_base_resolve_reverse;

	std::function<evdns_request *(evdns_base *, const struct in6_addr *, int,
	    evdns_callback_type, void *)> evdns_base_resolve_reverse_ipv6 =
	    ::evdns_base_resolve_reverse_ipv6;

	std::function<void(int, char, int, int, void *, void *)> evdns_reply_hook;

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

	//
	// libc functions (we should probably rename this class and this file)
	//

	std::function<int(int, const char *, void *)> inet_pton = ::inet_pton;

	std::function<const char *(int, const void *, char *, socklen_t)>
	    inet_ntop = ::inet_ntop;
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

class IghtEvbuffer : public ight::common::constraints::NonCopyable,
		public ight::common::constraints::NonMovable {

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
};

class IghtBuffereventSocket : public ight::common::constraints::NonCopyable,
		public ight::common::constraints::NonMovable {

	IghtLibevent *libevent = IghtGlobalLibevent::get();
	bufferevent *bev = NULL;

    public:
	IghtBuffereventSocket(IghtLibevent *lev = NULL) {
		if (lev != NULL)
			libevent = lev;
	}

	IghtBuffereventSocket(event_base *base, evutil_socket_t fd,
	    int options, IghtLibevent *lev = NULL) {
		if (lev != NULL)
			libevent = lev;
		if ((bev = libevent->bufferevent_socket_new(base, fd,
		    options)) == NULL)
			throw std::bad_alloc();
	}

	~IghtBuffereventSocket(void) {
		if (bev != NULL)
			libevent->bufferevent_free(bev);
	}

	operator bufferevent *(void) {
		if (bev == NULL)
			throw std::runtime_error("Accessing NULL bufferevent");
		return (bev);
	}

	IghtLibevent *get_libevent(void) {
		return (libevent);
	}
};

#endif  // LIBIGHT_COMMON_LIBEVENT_H
