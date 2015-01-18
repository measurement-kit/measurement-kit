/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#ifndef LIBIGHT_NET_CONNECTION_HPP
# define LIBIGHT_NET_CONNECTION_HPP

#include "common/constraints.hpp"
#include "common/error.h"
#include "common/pointer.hpp"
#include "common/poller.h"
#include "common/utils.hpp"

#include "net/buffer.hpp"

#include "protocols/dns.hpp"

#include <event2/bufferevent.h>
#include <event2/event.h>

#include <stdexcept>
#include <string.h>

struct IghtStringVector;

namespace ight {
namespace net {
namespace connection {

using namespace ight::common::constraints;
using namespace ight::common::pointer;
using namespace ight::protocols;

class ConnectionState {

	IghtBuffereventSocket bev;
	dns::Request dns_request;
	unsigned int connecting = 0;
	char *address = NULL;
	char *port = NULL;
	IghtStringVector *addrlist = NULL;
	char *family = NULL;
	IghtStringVector *pflist = NULL;
	unsigned int must_resolve_ipv4 = 0;
	unsigned int must_resolve_ipv6 = 0;
	SharedPointer<IghtDelayedCall> start_connect;

	// Libevent callbacks
	static void handle_read(bufferevent *, void *);
	static void handle_write(bufferevent *, void *);
	static void handle_event(bufferevent *, short, void *);

	// Functions used when connecting
	void connect_next(void);
	void handle_resolve(int, char, std::vector<std::string>);
	static void resolve(ConnectionState *);
	bool resolve_internal(char);

	std::function<void(void)> on_connect_fn = [](void) {
		/* nothing */
	};

	std::function<void(void)> on_ssl_fn = [](void) {
		/* nothing */
	};

	std::function<void(SharedPointer<IghtBuffer>)> on_data_fn = [](
			SharedPointer<IghtBuffer>) {
		/* nothing */
	};

	std::function<void(void)> on_flush_fn = [](void) {
		/* nothing */
	};

	std::function<void(IghtError)> on_error_fn = [](IghtError) {
		/* nothing */
	};

    public:
	ConnectionState(const char *, const char *, const char *,
	    evutil_socket_t = IGHT_SOCKET_INVALID);

	ConnectionState(ConnectionState&) = delete;
	ConnectionState& operator=(ConnectionState&) = delete;
	ConnectionState(ConnectionState&&) = delete;
	ConnectionState& operator=(ConnectionState&&) = delete;

	~ConnectionState(void);

	void on_connect(std::function<void(void)>&& fn) {
		on_connect_fn = std::move(fn);
	};

	void on_ssl(std::function<void(void)>&& fn) {
		on_ssl_fn = std::move(fn);
	};

	void on_data(std::function<void(SharedPointer<IghtBuffer>)>&& fn) {
		on_data_fn = std::move(fn);
		enable_read();
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

	void send(SharedPointer<IghtBuffer> data) {
		send(*data);
	}

	void send(IghtBuffer& data) {
		data >> bufferevent_get_output(bev);
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

class Connection : public NonCopyable, public NonMovable {

	ConnectionState *state = NULL;

    public:
	Connection(void) {
		/* nothing to do */
	}
	Connection(evutil_socket_t fd) {
		state = new ConnectionState("PF_UNSPEC", "0.0.0.0",
		    "0", fd);
	}
	Connection(const char *af, const char *a, const char *p) {
		state = new ConnectionState(af, a, p);
	}

	void close(void) {
		if (state == NULL)
			throw std::runtime_error("Invalid state");
		state->close();
	}

	~Connection(void) {
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

	void on_data(std::function<void(SharedPointer<IghtBuffer>)>&& fn) {
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

	void send(SharedPointer<IghtBuffer> sourcebuf) {
		if (state == NULL)
			throw std::runtime_error("Invalid state");
		state->send(sourcebuf);
	}

	void send(IghtBuffer& sourcebuf) {
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

}}}  // namespaces
#endif  /* LIBIGHT_NET_CONNECTION_HPP */
