/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#ifndef LIBIGHT_NET_CONNECTION_H
# define LIBIGHT_NET_CONNECTION_H
# ifdef __cplusplus

#include "common/constraints.hpp"
#include "common/error.h"
#include "common/pointer.hpp"
#include "common/poller.h"
#include "common/utils.hpp"

#include "protocols/dns.hpp"

#include <event2/bufferevent.h>
#include <event2/event.h>

#include <stdexcept>
#include <string.h>

struct IghtStringVector;
struct evbuffer;

class IghtConnectionState {

	IghtBuffereventSocket bev;
	ight::protocols::dns::Request dns_request;
	unsigned int connecting = 0;
	char *address = NULL;
	char *port = NULL;
	IghtStringVector *addrlist = NULL;
	char *family = NULL;
	IghtStringVector *pflist = NULL;
	unsigned int must_resolve_ipv4 = 0;
	unsigned int must_resolve_ipv6 = 0;
	ight::common::pointer::SharedPointer<IghtDelayedCall> start_connect;

	// Libevent callbacks
	static void handle_read(bufferevent *, void *);
	static void handle_write(bufferevent *, void *);
	static void handle_event(bufferevent *, short, void *);

	// Functions used when connecting
	void connect_next(void);
	void handle_resolve(int, char, std::vector<std::string>);
	static void resolve(IghtConnectionState *);
	bool resolve_internal(char);

	std::function<void(void)> on_connect_fn = [](void) {
		/* nothing */
	};

	std::function<void(void)> on_ssl_fn = [](void) {
		/* nothing */
	};

	std::function<void(evbuffer *)> on_data_fn = [](evbuffer *) {
		/* nothing */
	};

	std::function<void(void)> on_flush_fn = [](void) {
		/* nothing */
	};

	std::function<void(IghtError)> on_error_fn = [](IghtError) {
		/* nothing */
	};

    public:
	IghtConnectionState(const char *, const char *, const char *,
	    evutil_socket_t = IGHT_SOCKET_INVALID);

	IghtConnectionState(IghtConnectionState&) = delete;
	IghtConnectionState& operator=(IghtConnectionState&) = delete;
	IghtConnectionState(IghtConnectionState&&) = delete;
	IghtConnectionState& operator=(IghtConnectionState&&) = delete;

	~IghtConnectionState(void);

	void on_connect(std::function<void(void)>&& fn) {
		on_connect_fn = std::move(fn);
	};

	void on_ssl(std::function<void(void)>&& fn) {
		on_ssl_fn = std::move(fn);
	};

	void on_data(std::function<void(evbuffer *)>&& fn) {
		on_data_fn = std::move(fn);
	};

	void on_flush(std::function<void(void)>&& fn) {
		on_flush_fn = std::move(fn);
	};

	void on_error(std::function<void(IghtError)>&& fn) {
		on_error_fn = std::move(fn);
	};

	evutil_socket_t get_fileno(void) {
		return (bufferevent_getfd(this->bev));
	}

	void set_timeout(double timeout) {
		struct timeval tv, *tvp;
		tvp = ight_timeval_init(&tv, timeout);
		if (bufferevent_set_timeouts(this->bev, tvp, tvp) != 0) {
			throw std::runtime_error("cannot set timeout");
		}
	}

	void clear_timeout(void) {
		this->set_timeout(-1);
	}

	void start_tls(unsigned int) {
		throw std::runtime_error("not implemented");
	}

	void send(const void *base, size_t count) {
		if (base == NULL || count == 0) {
			throw std::runtime_error("invalid argument");
		}
		if (bufferevent_write(bev, base, count) != 0) {
			throw std::runtime_error("cannot write");
		}
	}

	void send(std::string data) {
		send(data.c_str(), data.length());
	}

	void send(evbuffer *data) {
		if (data == NULL) {
			throw std::runtime_error("invalid argument");
		}
		if (bufferevent_write_buffer(bev, data) != 0) {
			throw std::runtime_error("cannot write");
		}
	}

	void enable_read(void) {
		if (bufferevent_enable(this->bev, EV_READ) != 0) {
			throw std::runtime_error("cannot enable read");
		}
	}

	void disable_read(void) {
		if (bufferevent_disable(this->bev, EV_READ) != 0) {
			throw std::runtime_error("cannot disable read");
		}
	}

	void close(void);
};

class IghtConnection : public ight::common::constraints::NonCopyable,
		public ight::common::constraints::NonMovable {

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

	void close(void) {
		if (state == NULL)
			throw std::runtime_error("Invalid state");
		state->close();
	}

	~IghtConnection(void) {
		delete state;  /* delete handles NULL */
	}

	void on_connect(std::function<void(void)>&& fn) {
		if (state == NULL)
			throw std::runtime_error("Invalid state");
		state->on_connect(std::move(fn));
	};

	void on_ssl(std::function<void(void)>&& fn) {
		if (state == NULL)
			throw std::runtime_error("Invalid state");
		state->on_ssl(std::move(fn));
	};

	void on_data(std::function<void(evbuffer *)>&& fn) {
		if (state == NULL)
			throw std::runtime_error("Invalid state");
		state->on_data(std::move(fn));
	};

	void on_flush(std::function<void(void)>&& fn) {
		if (state == NULL)
			throw std::runtime_error("Invalid state");
		state->on_flush(std::move(fn));
	};

	void on_error(std::function<void(IghtError)>&& fn) {
		if (state == NULL)
			throw std::runtime_error("Invalid state");
		state->on_error(std::move(fn));
	};

	evutil_socket_t get_fileno(void) {
		if (state == NULL)
			throw std::runtime_error("Invalid state");
		return (state->get_fileno());
	}

	void set_timeout(double timeout) {
		if (state == NULL)
			throw std::runtime_error("Invalid state");
		state->set_timeout(timeout);
	}

	void clear_timeout(void) {
		if (state == NULL)
			throw std::runtime_error("Invalid state");
		state->clear_timeout();
	}

	void start_tls(unsigned int d) {
		if (state == NULL)
			throw std::runtime_error("Invalid state");
		state->start_tls(d);
	}

	void send(const char *base, size_t count) {
		if (state == NULL)
			throw std::runtime_error("Invalid state");
		state->send(base, count);
	}

	void send(std::string str) {
		if (state == NULL)
			throw std::runtime_error("Invalid state");
		state->send(str);
	}

	void send(evbuffer *sourcebuf) {
		if (state == NULL)
			throw std::runtime_error("Invalid state");
		state->send(sourcebuf);
	}

	void enable_read(void) {
		if (state == NULL)
			throw std::runtime_error("Invalid state");
		state->enable_read();
	}

	void disable_read(void) {
		if (state == NULL)
			throw std::runtime_error("Invalid state");
		state->disable_read();
	}
};

# endif  /* __cplusplus */
#endif  /* LIBIGHT_NET_CONNECTION_H */
