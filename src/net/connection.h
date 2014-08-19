/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#ifndef LIBIGHT_NET_CONNECTION_H
# define LIBIGHT_NET_CONNECTION_H
# ifdef __cplusplus

#include "common/error.h"
#include "common/poller.h"
#include "common/utils.h"

#include "net/ll2sock.h"

#include <event2/bufferevent.h>
#include <event2/event.h>

#include <string.h>

struct evbuffer;

struct IghtStringVector;

class IghtConnection {

	long long filedesc = IGHT_SOCKET_INVALID;
	bufferevent *bev = NULL;
	unsigned int closing = 0;
	unsigned int connecting = 0;
	unsigned int reading = 0;
	char *address = NULL;
	char *port = NULL;
	IghtStringVector *addrlist = NULL;
	char *family = NULL;
	IghtStringVector *pflist = NULL;
	unsigned int must_resolve_ipv4 = 0;
	unsigned int must_resolve_ipv6 = 0;
	IghtDelayedCall start_connect;

	// Private destructor because destruction may be delayed
	~IghtConnection(void);

	// Libevent callbacks
	static void handle_read(bufferevent *, void *);
	static void handle_write(bufferevent *, void *);
	static void handle_event(bufferevent *, short, void *);

	// Functions used by connect_hostname()
	void connect_next(void);
	static void handle_resolve(int, char, int, int,
	    void *, void *);
	static void resolve(void *);

    public:
	IghtConnection(long long);
	IghtConnection(const char *, const char *, const char *);

	std::function<void(void)> on_connect = [](void) {
		/* nothing */
	};

	std::function<void(void)> on_ssl = [](void) {
		/* nothing */
	};

	std::function<void(evbuffer *)> on_data = [](evbuffer *) {
		/* nothing */
	};

	std::function<void(void)> on_flush = [](void) {
		/* nothing */
	};

	std::function<void(IghtError)> on_error = [](IghtError) {
		/* nothing */
	};

	int set_timeout(double timeout) {
		struct timeval tv, *tvp;
		tvp = ight_timeval_init(&tv, timeout);
		return (bufferevent_set_timeouts(this->bev, tvp, tvp));
	}

	int clear_timeout(void) {
		return (this->set_timeout(-1));
	}

	int start_tls(unsigned int) {
		return (-1);		/* TODO: implement */
	}

	int write(const char *base, size_t count) {
		if (base == NULL || count == 0)
			return (-1);
		return (bufferevent_write(this->bev, base, count));
	}

	int puts(const char *str) {
		if (str == NULL)
			return (-1);
		return (this->write(str, strlen(str)));
	}

	int write_from(evbuffer *sourcebuf) {
		if (sourcebuf == NULL)
			return (-1);
		return (bufferevent_write_buffer(this->bev, sourcebuf));
	}

	int enable_read(void) {
		return (bufferevent_enable(this->bev, EV_READ));
	}

	int disable_read(void) {
		return (bufferevent_disable(this->bev, EV_READ));
	}

	void close(void);
};

# endif  /* __cplusplus */
#endif  /* LIBIGHT_NET_CONNECTION_H */
