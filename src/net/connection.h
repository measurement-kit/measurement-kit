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
#include "common/pointer.hpp"
#include "common/poller.h"
#include "common/utils.hpp"

#include <event2/bufferevent.h>
#include <event2/event.h>

#include <stdexcept>
#include <string.h>

struct IghtStringVector;
struct evbuffer;

class IghtConnectionState {

	evutil_socket_t filedesc = IGHT_SOCKET_INVALID;
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
	ight::common::pointer::SharedPointer<IghtDelayedCall> start_connect;

	// Private destructor because destruction may be delayed
	~IghtConnectionState(void);

	// Libevent callbacks
	static void handle_read(bufferevent *, void *);
	static void handle_write(bufferevent *, void *);
	static void handle_event(bufferevent *, short, void *);

	// Functions used when connecting
	void connect_next(void);
	static void handle_resolve(int, char, int, int,
	    void *, void *);
	static void resolve(IghtConnectionState *);

    public:
	IghtConnectionState(const char *, const char *, const char *,
	    evutil_socket_t = IGHT_SOCKET_INVALID);

	IghtConnectionState(IghtConnectionState&) = delete;
	IghtConnectionState& operator=(IghtConnectionState&) = delete;
	IghtConnectionState(IghtConnectionState&&) = delete;
	IghtConnectionState& operator=(IghtConnectionState&&) = delete;

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

	evutil_socket_t get_fileno(void) {
		return (this->filedesc);
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

class IghtConnection {
	IghtConnectionState *state = NULL;

    public:
	IghtConnection(void) {
		/* nothing to do */
	}
	IghtConnection(evutil_socket_t fd) {
		state = new IghtConnectionState("PF_UNSPEC", "0.0.0.0",
		    "0", fd);
	}
	IghtConnection(const char *af, const char *a, const char *p) {
		state = new IghtConnectionState(af, a, p);
	}

	/* We don't want multiple copies of `state` */
	IghtConnection(IghtConnection&) = delete;
	IghtConnection& operator=(IghtConnection&) = delete;

	IghtConnection(IghtConnection&& other) {
		std::swap(state, other.state);
	}
	IghtConnection& operator=(IghtConnection&& other) {
		std::swap(state, other.state);
		return (*this);
	}

	void close(void) {
		if (state == NULL)
			return;
		state->close();
		state = NULL;		/* Idempotent */
	}

	~IghtConnection(void) {
		close();
	}

	void on_connect(std::function<void(void)>&& fn) {
		if (state == NULL)
			throw std::runtime_error("Invalid state");
		state->on_connect = std::move(fn);
	};

	void on_ssl(std::function<void(void)>&& fn) {
		if (state == NULL)
			throw std::runtime_error("Invalid state");
		state->on_ssl = std::move(fn);
	};

	void on_data(std::function<void(evbuffer *)>&& fn) {
		if (state == NULL)
			throw std::runtime_error("Invalid state");
		state->on_data = std::move(fn);
	};

	void on_flush(std::function<void(void)>&& fn) {
		if (state == NULL)
			throw std::runtime_error("Invalid state");
		state->on_flush = std::move(fn);
	};

	void on_error(std::function<void(IghtError)>&& fn) {
		if (state == NULL)
			throw std::runtime_error("Invalid state");
		state->on_error = std::move(fn);
	};

	evutil_socket_t get_fileno(void) {
		if (state == NULL)
			throw std::runtime_error("Invalid state");
		return (state->get_fileno());
	}

	int set_timeout(double timeout) {
		if (state == NULL)
			throw std::runtime_error("Invalid state");
		return (state->set_timeout(timeout));
	}

	int clear_timeout(void) {
		if (state == NULL)
			throw std::runtime_error("Invalid state");
		return (state->clear_timeout());
	}

	int start_tls(unsigned int d) {
		if (state == NULL)
			throw std::runtime_error("Invalid state");
		return (state->start_tls(d));
	}

	int write(const char *base, size_t count) {
		if (state == NULL)
			throw std::runtime_error("Invalid state");
		return (state->write(base, count));
	}

	int puts(const char *str) {
		if (state == NULL)
			throw std::runtime_error("Invalid state");
		return (state->puts(str));
	}

	int write_from(evbuffer *sourcebuf) {
		if (state == NULL)
			throw std::runtime_error("Invalid state");
		return (state->write_from(sourcebuf));
	}

	int enable_read(void) {
		if (state == NULL)
			throw std::runtime_error("Invalid state");
		return (state->enable_read());
	}

	int disable_read(void) {
		if (state == NULL)
			throw std::runtime_error("Invalid state");
		return (state->disable_read());
	}
};

# endif  /* __cplusplus */
#endif  /* LIBIGHT_NET_CONNECTION_H */
