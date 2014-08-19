/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#ifndef LIBIGHT_NET_CONNECTION_H
# define LIBIGHT_NET_CONNECTION_H
# ifdef __cplusplus

#include "common/poller.h"
#include "common/utils.h"

#include <event2/bufferevent.h>
#include <event2/event.h>

struct evbuffer;

struct IghtStringVector;
struct IghtProtocol;

class IghtConnection {

	long long filedesc;
	bufferevent *bev;
	IghtProtocol *protocol;
	unsigned int closing;
	unsigned int connecting;
	unsigned int reading;
	char *address;
	char *port;
	IghtStringVector *addrlist;
	char *family;
	IghtStringVector *pflist;
	unsigned int must_resolve_ipv4;
	unsigned int must_resolve_ipv6;
	IghtDelayedCall start_connect;

	IghtConnection(void);

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
	static IghtConnection *attach(IghtProtocol *, long long);

	static IghtConnection *connect(IghtProtocol *, const char *,
	    const char *, const char *);

	IghtProtocol *get_protocol(void) {
		return (this->protocol);
	}

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
