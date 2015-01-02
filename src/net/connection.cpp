/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#include <arpa/inet.h>

#include <new>
#include <stdlib.h>

#include <event2/dns.h>

#include "common/log.h"
#include "common/stringvector.h"
#include "net/connection.h"

IghtConnectionState::~IghtConnectionState(void)
{
	/*
	 * TODO: switch to RAII.
	 */

	if (this->filedesc != IGHT_SOCKET_INVALID)
		(void) evutil_closesocket((evutil_socket_t) this->filedesc);

	if (this->bev != NULL)
		bufferevent_free(this->bev);

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
IghtConnectionState::handle_read(bufferevent *bev, void *opaque)
{
	auto self = (IghtConnectionState *) opaque;

	(void) bev;  // Suppress warning about unused variable

	self->reading = 1;
	self->on_data(bufferevent_get_input(self->bev));
	self->reading = 0;

	if (self->closing)
		delete (self);
}

void
IghtConnectionState::handle_write(bufferevent *bev, void *opaque)
{
	auto self = (IghtConnectionState *) opaque;
	(void) bev;  // Suppress warning about unused variable
	self->on_flush();
}

void
IghtConnectionState::handle_event(bufferevent *bev, short what, void *opaque)
{
	auto self = (IghtConnectionState *) opaque;

	(void) bev;  // Suppress warning about unused variable

	if (self->connecting && self->closing) {
		delete (self);
		return;
	}

	if (what & BEV_EVENT_CONNECTED) {
		self->connecting = 0;
		self->on_connect();
		return;
	}

	if (what & BEV_EVENT_EOF) {
		self->on_error(IghtError(0));
		return;
	}

	if (self->connecting) {
		ight_info("connection::handle_event - try connect next");
		self->connect_next();
		return;
	}

	// TODO: also handle the timeout

	self->on_error(IghtError(-1));
}

IghtConnectionState::IghtConnectionState(const char *family, const char *address,
    const char *port, evutil_socket_t filenum)
{
	auto evbase = ight_get_global_event_base();
	auto poller = ight_get_global_poller();

	filenum = ight_socket_normalize_if_invalid(filenum);

	/*
	 * TODO: switch to RAII.
	 */

	// filedesc: if valid, it is set on success only

	if ((this->bev = bufferevent_socket_new(evbase, (evutil_socket_t)
	    filenum, BEV_OPT_DEFER_CALLBACKS)) == NULL)
		throw std::bad_alloc();

	// closing: nothing to be done

	if (!ight_socket_valid(filenum))
		this->connecting = 1;

	// reading: nothing to be done

	if ((this->address = strdup(address)) == NULL) {
		bufferevent_free(this->bev);
		throw std::bad_alloc();
	}

	if ((this->port = strdup(port)) == NULL) {
		bufferevent_free(this->bev);
		free(this->address);
		throw std::bad_alloc();
	}

	if ((this->addrlist = new (std::nothrow) IghtStringVector(poller,
	    16)) == NULL) {
		bufferevent_free(this->bev);
		free(this->address);
		free(this->port);
		throw std::bad_alloc();
	}

	if ((this->family = strdup(family)) == NULL) {
		bufferevent_free(this->bev);
		free(this->address);
		free(this->port);
		delete (this->addrlist);
		throw std::bad_alloc();
	}

	if ((this->pflist = new (std::nothrow) IghtStringVector(poller,
	    16)) == NULL) {
		bufferevent_free(this->bev);
		free(this->address);
		free(this->port);
		delete (this->addrlist);
		free(this->family);
		throw std::bad_alloc();
	}

	// must_resolve_ipv4: if connecting, set later by this->resolve()
	// must_resolve_ipv6: if connecting, set later by this->resolve()

	/*
	 * The following makes this non copyable and non movable.
	 */
	bufferevent_setcb(this->bev, this->handle_read, this->handle_write,
	    this->handle_event, this);

	if (!ight_socket_valid(filenum))
		this->start_connect = std::make_shared<IghtDelayedCall>(0.0,
		    std::bind(this->resolve, this));
	else
		this->filedesc = filenum;	/* Own the socket on success */
}

void
IghtConnectionState::connect_next(void)
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
	this->on_error(IghtError(-2));
}

void
IghtConnectionState::handle_resolve(int result, char type, int count,
    int ttl, void *addresses, void *opaque)
{
	auto self = (IghtConnectionState *) opaque;
	const char *_family;
	const char *p;
	int error, family, size;
	char string[128];

	(void) ttl;

	ight_info("handle_resolve - enter");

	if (!self->connecting)
		abort();

	if (self->closing) {
		ight_info("handle_resolve - delayed close");
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
			self->on_error(IghtError(-3));
			return;
		}
	}

    finally:
	auto dns_base = ight_get_global_evdns_base();

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
IghtConnectionState::resolve(IghtConnectionState *self)
{
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
			self->on_error(IghtError(-4));
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
			self->on_error(IghtError(-4));
			return;
		}
		self->connect_next();
		return;
	}

	auto dns_base = ight_get_global_evdns_base();

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
		self->on_error(IghtError(-5));
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
		self->on_error(IghtError(-6));
		return;
	}

	// Arrange for the next resolve operation that we will need
	if (strcmp(self->family, "PF_UNSPEC") == 0)
		self->must_resolve_ipv6 = 1;
	else if (strcmp(self->family, "PF_UNSPEC6") == 0)
		self->must_resolve_ipv4 = 1;
}

void
IghtConnectionState::close(void)
{
	this->closing = 1;
	if (this->reading != 0 || this->connecting != 0)
		return;
	delete this;
}
