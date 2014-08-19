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

	(void) bev;  // Suppress warning about unused variable

	self->reading = 1;
	self->protocol->on_data(bufferevent_get_input(self->bev));
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
	evbase = poller->get_event_base();
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
	evdns_base *dns_base = poller->get_evdns_base();
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

	evdns_base *dns_base = poller->get_evdns_base();
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
IghtConnection::connect(IghtProtocol *proto,
    const char *family, const char *address, const char *port)
{
	event_base *evbase;
	IghtPoller *poller;
	IghtConnection *self;

	if (proto == NULL || family == NULL || address == NULL || port == NULL)
		abort();
	poller = proto->get_poller();
	if (poller == NULL)
		abort();
	evbase = poller->get_event_base();
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

	self->start_connect = IghtDelayedCall(0.0, std::bind(
	    self->resolve, self));

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
IghtConnection::write_from(evbuffer *sourcebuf)
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
