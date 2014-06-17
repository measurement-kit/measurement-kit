/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#include <arpa/inet.h>

#include <limits.h>
#include <new>
#include <stdlib.h>
#include <string.h>

#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/dns.h>
#include <event2/event.h>

#include "common/stringvector.h"
#include "common/poller.h"
#include "common/utils.h"
#include "common/log.h"

#include "net/connection.h"
#include "net/protocol.h"
#include "net/ll2sock.h"

IghtConnection::IghtConnection(void)
{
	this->filedesc = IGHT_SOCKET_INVALID;
	this->bev = NULL;
	this->protocol = NULL;
	this->readbuf = NULL;
	this->closing = 0;
	this->connecting = 0;
	this->reading = 0;
	this->address = NULL;
	this->port = NULL;
	this->addrlist = NULL;
	this->family = NULL;
	this->pflist = NULL;
	this->must_resolve_ipv4 = 0;
	this->must_resolve_ipv6 = 0;
}

IghtConnection::~IghtConnection(void)
{
	if (this->filedesc != IGHT_SOCKET_INVALID)
		(void) evutil_closesocket((evutil_socket_t) this->filedesc);

	if (this->bev != NULL)
		bufferevent_free(this->bev);

	// protocol: should already be dead

	if (this->readbuf != NULL)
		evbuffer_free(this->readbuf);

	// closing: nothing to be done
	// connecting: nothing to be done
	// reading: nothing to be done

	if (this->address != NULL)
		free(this->address);
	if (this->port != NULL)
		free(this->port);
	if (this->addrlist != NULL)
		delete (this->addrlist);

	if (this->family != NULL)
		free(this->family);
	if (this->pflist != NULL)
		delete (this->pflist);

	// must_resolve_ipv4: nothing to be done
	// must_resolve_ipv6: nothing to be done
}

void
IghtConnection::handle_read(bufferevent *bev, void *opaque)
{
	IghtConnection *self = (IghtConnection *) opaque;
	int result;

	(void) bev;  // Suppress warning about unused variable

	result = bufferevent_read_buffer(self->bev, self->readbuf);
	if (result != 0) {
		self->protocol->on_error();
		return;
	}

	self->reading = 1;
	self->protocol->on_data();
	self->reading = 0;

	if (self->closing)
		delete (self);
}

void
IghtConnection::handle_write(bufferevent *bev, void *opaque)
{
	IghtConnection *self = (IghtConnection *) opaque;
	(void) bev;  // Suppress warning about unused variable
	self->protocol->on_flush();
}

void
IghtConnection::handle_event(bufferevent *bev, short what, void *opaque)
{
	IghtConnection *self = (IghtConnection *) opaque;

	(void) bev;  // Suppress warning about unused variable

	if (self->connecting && self->closing) {
		delete (self);
		return;
	}

	if (what & BEV_EVENT_CONNECTED) {
		self->connecting = 0;
		int result = bufferevent_enable(self->bev, EV_READ);
		if (result != 0) {
			self->protocol->on_error();
			return;
		}
		self->protocol->on_connect();
		return;
	}

	if (what & BEV_EVENT_EOF) {
		self->protocol->on_eof();
		return;
	}

	if (self->connecting) {
		ight_info("connection::handle_event - try connect next");
		self->connect_next();
		return;
	}

	// TODO: also handle the timeout

	self->protocol->on_error();
}

IghtConnection *
IghtConnection::attach(IghtProtocol *proto, long long filenum)
{
	event_base *evbase;
	IghtPoller *poller;
	IghtConnection *self;

	if (proto == NULL)
		abort();
	poller = proto->get_poller();
	if (poller == NULL)
		abort();
	evbase = IghtPoller_get_event_base(poller);
	if (evbase == NULL)
		abort();

	if (!ight_socket_valid(filenum))
		return (NULL);

	self = new (std::nothrow) IghtConnection();
	if (self == NULL)
		return (NULL);

	// filedesc: only set on success

	self->bev = bufferevent_socket_new(evbase, (evutil_socket_t)filenum,
	    BEV_OPT_DEFER_CALLBACKS);
	if (self->bev == NULL) {
		delete self;
		return (NULL);
	}

	self->protocol = proto;

	self->readbuf = evbuffer_new();
	if (self->readbuf == NULL) {
		delete self;
		return (NULL);
	}

	// closing: nothing to be done
	// connecting: nothing to be done
	// reading: nothing to be done

	self->address = strdup("0.0.0.0");
	if (self->address == NULL) {
		delete self;
		return (NULL);
	}

	self->port = strdup("0");
	if (self->port == NULL) {
		delete self;
		return (NULL);
	}

	self->addrlist = new (std::nothrow) IghtStringVector(poller, 16);
	if (self->addrlist == NULL) {
		delete self;
		return (NULL);
	}

	self->family = strdup("PF_UNSPEC");
	if (self->family == NULL) {
		delete self;
		return (NULL);
	}

	self->pflist = new (std::nothrow) IghtStringVector(poller, 16);
	if (self->pflist == NULL) {
		delete self;
		return (NULL);
	}

	// must_resolve_ipv4: nothing to be done
	// must_resolve_ipv6: nothing to be done

	bufferevent_setcb(self->bev, self->handle_read, self->handle_write,
	    self->handle_event, self);

	// Only own the filedesc when we know there were no errors
	self->filedesc = filenum;
	return (self);
}

IghtConnection *
IghtConnection::connect(IghtProtocol *proto, const char *family,
    const char *address, const char *port)
{
	event_base *evbase;
	IghtPoller *poller;
	int result;
	IghtConnection *self;
	struct sockaddr_storage storage;
	socklen_t total;

	if (proto == NULL || family == NULL || address == NULL || port == NULL)
		abort();
	poller = proto->get_poller();
	if (poller == NULL)
		abort();
	evbase = IghtPoller_get_event_base(poller);
	if (evbase == NULL)
		abort();

	result = ight_storage_init(&storage, &total, family, address, port);
	if (result != 0)
		return (NULL);

	self = new (std::nothrow) IghtConnection();
	if (self == NULL)
		return (NULL);

	self->filedesc = ight_socket_create(storage.ss_family,
	    SOCK_STREAM, 0);
	if (self->filedesc == IGHT_SOCKET_INVALID) {
		delete self;
		return (NULL);
	}

	self->bev = bufferevent_socket_new(evbase,
	    (evutil_socket_t)self->filedesc, BEV_OPT_DEFER_CALLBACKS);
	if (self->bev == NULL) {
		delete self;
		return (NULL);
	}

	self->protocol = proto;

	self->readbuf = evbuffer_new();
	if (self->readbuf == NULL) {
		delete self;
		return (NULL);
	}

	// closing: nothing to be done

	self->connecting = 1;

	// reading: nothing to be done

	self->address = strdup(address);
	if (self->address == NULL) {
		delete self;
		return (NULL);
	}

	self->port = strdup(port);
	if (self->port == NULL) {
		delete self;
		return (NULL);
	}

	self->addrlist = new (std::nothrow) IghtStringVector(poller, 16);
	if (self->addrlist == NULL) {
		delete self;
		return (NULL);
	}

	self->family = strdup(family);
	if (self->family == NULL) {
		delete self;
		return (NULL);
	}

	self->pflist = new (std::nothrow) IghtStringVector(poller, 16);
	if (self->pflist == NULL) {
		delete self;
		return (NULL);
	}

	// must_resolve_ipv4: nothing to be done
	// must_resolve_ipv6: nothing to be done

	bufferevent_setcb(self->bev, self->handle_read, self->handle_write,
	    self->handle_event, self);

	result = bufferevent_socket_connect(self->bev, (struct sockaddr *)
	    &storage, (int) total);
	if (result != 0) {
		delete self;
		return (NULL);
	}

	return (self);
}

void
IghtConnection::connect_next(void)
{
	const char *address;
	int error;
	const char *family;
	struct sockaddr_storage storage;
	socklen_t total;

	ight_info("connect_next - enter");

	for (;;) {
		address = this->addrlist->get_next();
		if (address == NULL) {
			ight_warn("connect_next - no more available addrs");
			break;
		}
		family = this->pflist->get_next();
		if (family == NULL)
			abort();

		ight_info("connect_next - %s %s", family, address);

		error = ight_storage_init(&storage, &total, family,
		    address, this->port);
		if (error != 0)
			continue;

		this->filedesc = ight_socket_create(storage.ss_family,
		    SOCK_STREAM, 0);
		if (this->filedesc == IGHT_SOCKET_INVALID)
			continue;

		error = bufferevent_setfd(this->bev, (evutil_socket_t)
		    this->filedesc);
		if (error != 0) {
			(void) evutil_closesocket(this->filedesc);
			this->filedesc = IGHT_SOCKET_INVALID;
			continue;
		}

		error = bufferevent_socket_connect(this->bev, (struct
		    sockaddr *) &storage, (int) total);
		if (error != 0) {
			(void) evutil_closesocket(this->filedesc);
			this->filedesc = IGHT_SOCKET_INVALID;
			error = bufferevent_setfd(this->bev,
			    IGHT_SOCKET_INVALID);
			if (error != 0) {
				ight_warn("connect_next - internal error");
				break;
			}
			continue;
		}

		ight_info("connect_next - ok");
		return;
	}

	this->connecting = 0;
	this->protocol->on_error();
}

void
IghtConnection::handle_resolve(int result, char type, int count,
    int ttl, void *addresses, void *opaque)
{
	IghtConnection *self = (IghtConnection *) opaque;
	const char *_family;
	const char *p;
	int error, family, size;
	char string[128];

	(void) ttl;

	ight_info("handle_resolve - enter");

	if (!self->connecting)
		abort();

	if (self->closing) {
		delete (self);
		return;
	}

	if (result != DNS_ERR_NONE)
		goto finally;

	switch (type) {
	case DNS_IPv4_A:
		ight_info("handle_resolve - IPv4");
		family = PF_INET;
		_family = "PF_INET";
		size = 4;
		break;
	case DNS_IPv6_AAAA:
		ight_info("handle_resolve - IPv6");
		family = PF_INET6;
		_family = "PF_INET6";
		size = 16;
		break;
	default:
		abort();
	}

	while (--count >= 0) {
		if (count > INT_MAX / size) {
			continue;
		}
		// Note: address already in network byte order
		p = inet_ntop(family, (char *)addresses + count * size,
		    string, sizeof (string));
		if (p == NULL) {
			ight_warn("handle_resolve - inet_ntop() failed");
			continue;
		}
		ight_info("handle_resolve - address %s", p);
		error = self->addrlist->append(string);
		if (error != 0) {
			ight_warn("handle_resolve - cannot append");
			continue;
		}
		ight_info("handle_resolve - family %s", _family);
		error = self->pflist->append(_family);
		if (error != 0) {
			ight_warn("handle_resolve - cannot append");
			// Oops the two vectors are not in sync anymore now
			self->connecting = 0;
			self->protocol->on_error();
			return;
		}
	}

    finally:
	IghtPoller *poller = self->protocol->get_poller();
	if (poller == NULL)
		abort();
	evdns_base *dns_base = IghtPoller_get_evdns_base(poller);
	if (dns_base == NULL)
		abort();
	if (self->must_resolve_ipv6) {
		self->must_resolve_ipv6 = 0;
		evdns_request *request = evdns_base_resolve_ipv6(dns_base,
		    self->address, DNS_QUERY_NO_SEARCH, self->handle_resolve,
		    self);
		if (request != NULL)
			return;
		/* FALLTHROUGH */
	}
	if (self->must_resolve_ipv4) {
		self->must_resolve_ipv4 = 0;
		evdns_request *request = evdns_base_resolve_ipv4(dns_base,
		    self->address, DNS_QUERY_NO_SEARCH, self->handle_resolve,
		    self);
		if (request != NULL)
			return;
		/* FALLTHROUGH */
	}
	self->connect_next();
}

void
IghtConnection::resolve(void *opaque)
{
	IghtConnection *self = (IghtConnection *) opaque;
	struct sockaddr_storage storage;
	int result;

	if (!self->connecting)
		abort();

	if (self->closing) {
		delete (self);
		return;
	}

	// If self->address is a valid IPv4 address, connect directly
	memset(&storage, 0, sizeof (storage));
	result = inet_pton(PF_INET, self->address, &storage);
	if (result == 1) {
		ight_info("resolve - address %s", self->address);
		ight_info("resolve - family PF_INET");
		if (self->addrlist->append(self->address) != 0 ||
		    self->pflist->append("PF_INET") != 0) {
			ight_warn("resolve - cannot append");
			self->connecting = 0;
			self->protocol->on_error();
			return;
		}
		self->connect_next();
		return;
	}

	// If self->address is a valid IPv6 address, connect directly
	memset(&storage, 0, sizeof (storage));
	result = inet_pton(PF_INET6, self->address, &storage);
	if (result == 1) {
		ight_info("resolve - address %s", self->address);
		ight_info("resolve - family PF_INET6");
		if (self->addrlist->append(self->address) != 0 ||
		    self->pflist->append("PF_INET6") != 0) {
			ight_warn("resolve - cannot append");
			self->connecting = 0;
			self->protocol->on_error();
			return;
		}
		self->connect_next();
		return;
	}

	IghtPoller *poller = self->protocol->get_poller();
	if (poller == NULL)
		abort();

	evdns_base *dns_base = IghtPoller_get_evdns_base(poller);
	if (dns_base == NULL)
		abort();

	// Note: PF_UNSPEC6 means that we try with IPv6 first
	if (strcmp(self->family, "PF_INET") == 0)
		self->must_resolve_ipv4 = 1;
	else if (strcmp(self->family, "PF_INET6") == 0)
		self->must_resolve_ipv6 = 1;
	else if (strcmp(self->family, "PF_UNSPEC") == 0)
		self->must_resolve_ipv4 = 1;
	else if (strcmp(self->family, "PF_UNSPEC6") == 0)
		self->must_resolve_ipv6 = 1;
	else {
		ight_warn("connection::resolve - invalid PF_xxx");
		self->connecting = 0;
		self->protocol->on_error();
		return;
	}

	evdns_request *request;

	if (self->must_resolve_ipv4) {
		self->must_resolve_ipv4 = 0;
		request = evdns_base_resolve_ipv4(dns_base, self->address,
		    DNS_QUERY_NO_SEARCH, self->handle_resolve, self);
	} else {
		self->must_resolve_ipv6 = 0;
		request = evdns_base_resolve_ipv6(dns_base, self->address,
		    DNS_QUERY_NO_SEARCH, self->handle_resolve, self);
	}
	if (request == NULL) {
		self->connecting = 0;
		self->protocol->on_error();
		return;
	}

	// Arrange for the next resolve operation that we will need
	if (strcmp(self->family, "PF_UNSPEC") == 0)
		self->must_resolve_ipv6 = 1;
	else if (strcmp(self->family, "PF_UNSPEC6") == 0)
		self->must_resolve_ipv4 = 1;
}

IghtConnection *
IghtConnection::connect_hostname(IghtProtocol *proto,
    const char *family, const char *address, const char *port)
{
	event_base *evbase;
	IghtPoller *poller;
	int result;
	IghtConnection *self;

	if (proto == NULL || family == NULL || address == NULL || port == NULL)
		abort();
	poller = proto->get_poller();
	if (poller == NULL)
		abort();
	evbase = IghtPoller_get_event_base(poller);
	if (evbase == NULL)
		abort();

	self = new (std::nothrow) IghtConnection();
	if (self == NULL)
		return (NULL);

	// filedesc: nothing to be done

	self->bev = bufferevent_socket_new(evbase,
	    (evutil_socket_t) IGHT_SOCKET_INVALID,
	    BEV_OPT_DEFER_CALLBACKS);
	if (self->bev == NULL) {
		delete self;
		return (NULL);
	}

	self->protocol = proto;

	self->readbuf = evbuffer_new();
	if (self->readbuf == NULL) {
		delete self;
		return (NULL);
	}

	// closing: nothing to be done

	self->connecting = 1;

	// reading: nothing to be done

	self->address = strdup(address);
	if (self->address == NULL) {
		delete self;
		return (NULL);
	}

	self->port = strdup(port);
	if (self->port == NULL) {
		delete self;
		return (NULL);
	}

	self->addrlist = new (std::nothrow) IghtStringVector(poller, 16);
	if (self->addrlist == NULL) {
		delete self;
		return (NULL);
	}

	self->family = strdup(family);
	if (self->family == NULL) {
		delete self;
		return (NULL);
	}

	self->pflist = new (std::nothrow) IghtStringVector(poller, 16);
	if (self->pflist == NULL) {
		delete self;
		return (NULL);
	}

	// must_resolve_ipv4: set later by self->resolve()
	// must_resolve_ipv6: set later by self->resolve()

	bufferevent_setcb(self->bev, self->handle_read, self->handle_write,
	    self->handle_event, self);

	result = IghtPoller_sched(poller, 0.0, self->resolve, self);
	if (result != 0) {
		delete self;
		return (NULL);
	}

	return (self);
}

IghtProtocol *
IghtConnection::get_protocol(void)
{
	return (this->protocol);
}

int
IghtConnection::set_timeout(double timeout)
{
	struct timeval tv, *tvp;
	tvp = ight_timeval_init(&tv, timeout);
	return (bufferevent_set_timeouts(this->bev, tvp, tvp));
}

int
IghtConnection::clear_timeout(void)
{
	return (this->set_timeout(-1));
}

int
IghtConnection::start_tls(unsigned server_side)
{
	(void) server_side;

	return (-1);  // TODO: implement
}

int
IghtConnection::read(char *base, size_t count)
{
	if (base == NULL || count == 0 || count > INT_MAX)
		return (-1);

	return (evbuffer_remove(this->readbuf, base, count));
}

int
IghtConnection::readline(char *base, size_t count)
{
	size_t eol_length = 0;
	evbuffer_ptr result = evbuffer_search_eol(this->readbuf,
	    NULL, &eol_length, EVBUFFER_EOL_CRLF);
	if (result.pos < 0) {
		if (evbuffer_get_length(this->readbuf) > count)
			return (-1);  /* line too long */
		return (0);
	}

	if ((size_t) result.pos > SSIZE_MAX - eol_length)
		return (-1);
	result.pos += eol_length;

	if ((size_t) result.pos > count)
		return (-1);  /* line too long */

	int llen = this->read(base, (size_t) result.pos);
	if (llen < 0)
		return (-1);

	return (llen);
}

int
IghtConnection::readn(char *base, size_t count)
{
	if (base == NULL || count == 0 || count > INT_MAX)
		return (-1);
	if (evbuffer_get_length(this->readbuf) < count)
		return (0);

	return (evbuffer_remove(this->readbuf, base, count));
}

int
IghtConnection::discardn(size_t count)
{
	if (count == 0 || count > INT_MAX)
		return (-1);
	if (evbuffer_get_length(this->readbuf) < count)
		return (0);

	return (evbuffer_drain(this->readbuf, count));
}

int
IghtConnection::write(const char *base, size_t count)
{
	if (base == NULL || count == 0)
		return (-1);

	return (bufferevent_write(this->bev, base, count));
}

int
IghtConnection::puts(const char *str)
{
	if (str == NULL)
		return (-1);

	return (this->write(str, strlen(str)));
}

#define N_EXTENTS 2

int
IghtConnection::write_rand_evbuffer(struct evbuffer *evbuf, size_t count)
{
        struct evbuffer_iovec vec[N_EXTENTS];
        int n_extents, i;

        n_extents = evbuffer_reserve_space(evbuf, count, vec, N_EXTENTS);
        if (n_extents < 0 || n_extents > N_EXTENTS)
                return (-1);

        for (i = 0; i < n_extents; i++) {
                if (count <= vec[i].iov_len) {
                        evutil_secure_rng_get_bytes(vec[i].iov_base, count);
                        vec[i].iov_len = count;
                        break;
                }

                evutil_secure_rng_get_bytes(vec[i].iov_base, vec[i].iov_len);
                count -= vec[i].iov_len;
        }

        if (i >= n_extents)
                return (-1);  // Should not happen

        return (evbuffer_commit_space(evbuf, vec, i + 1));
}

int
IghtConnection::write_rand(size_t count)
{
	struct evbuffer *evbuf_output;
	
	if (count == 0)
		return (-1);

	evbuf_output = bufferevent_get_output(this->bev);
	if (evbuf_output == NULL)
		return (-1);

	return (this->write_rand_evbuffer(evbuf_output, count));
}

int
IghtConnection::write_readbuf(const char *base, size_t count)
{
	if (base == NULL || count == 0)
		return (-1);

	return (evbuffer_add(this->readbuf, (void *) base, count));
}

int
IghtConnection::puts_readbuf(const char *str)
{
	if (str == NULL)
		return (-1);

	return (this->write_readbuf(str, strlen(str)));
}

int
IghtConnection::write_rand_readbuf(size_t count)
{
	if (count == 0)
		return (-1);

	return (write_rand_evbuffer(this->readbuf, count));
}

int
IghtConnection::read_into_(evbuffer *destbuf)
{
	if (destbuf == NULL)
		return (-1);

	return (evbuffer_add_buffer(destbuf, this->readbuf));
}

int
IghtConnection::write_from_(evbuffer *sourcebuf)
{
	if (sourcebuf == NULL)
		return (-1);

	return (bufferevent_write_buffer(this->bev, sourcebuf));
}

int
IghtConnection::enable_read(void)
{
	return (bufferevent_enable(this->bev, EV_READ));
}

int
IghtConnection::disable_read(void)
{
	return (bufferevent_disable(this->bev, EV_READ));
}

void
IghtConnection::close(void)
{
	this->closing = 1;
	if (this->reading != 0 || this->connecting != 0)
		return;
	delete this;
}
