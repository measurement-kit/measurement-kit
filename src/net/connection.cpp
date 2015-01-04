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

using namespace ight::net::connection;
using namespace ight::protocols;

ConnectionState::~ConnectionState(void)
{
	/*
	 * TODO: switch to RAII.
	 */

	// connecting: nothing to be done

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
ConnectionState::handle_read(bufferevent *bev, void *opaque)
{
	auto self = (ConnectionState *) opaque;
	(void) bev;  // Suppress warning about unused variable
	self->on_data_fn(std::make_shared<IghtBuffer>(
			bufferevent_get_input(self->bev)));
}

void
ConnectionState::handle_write(bufferevent *bev, void *opaque)
{
	auto self = (ConnectionState *) opaque;
	(void) bev;  // Suppress warning about unused variable
	self->on_flush_fn();
}

void
ConnectionState::handle_event(bufferevent *bev, short what, void *opaque)
{
	auto self = (ConnectionState *) opaque;

	(void) bev;  // Suppress warning about unused variable

	if (what & BEV_EVENT_CONNECTED) {
		self->connecting = 0;
		self->on_connect_fn();
		return;
	}

	if (what & BEV_EVENT_EOF) {
		self->on_error_fn(IghtError(0));
		return;
	}

	if (self->connecting) {
		ight_info("connection::handle_event - try connect next");
		self->connect_next();
		return;
	}

	// TODO: also handle the timeout

	self->on_error_fn(IghtError(-1));
}

ConnectionState::ConnectionState(const char *family, const char *address,
    const char *port, evutil_socket_t filenum)
{
	auto evbase = ight_get_global_event_base();
	auto poller = ight_get_global_poller();

	filenum = ight_socket_normalize_if_invalid(filenum);

	/*
	 * TODO: switch to RAII.
	 */

	this->bev.make(evbase, filenum, BEV_OPT_DEFER_CALLBACKS|
	    BEV_OPT_CLOSE_ON_FREE);

	// closing: nothing to be done

	if (!ight_socket_valid(filenum))
		this->connecting = 1;

	if ((this->address = strdup(address)) == NULL) {
		throw std::bad_alloc();
	}

	if ((this->port = strdup(port)) == NULL) {
		free(this->address);
		throw std::bad_alloc();
	}

	if ((this->addrlist = new (std::nothrow) IghtStringVector(poller,
	    16)) == NULL) {
		free(this->address);
		free(this->port);
		throw std::bad_alloc();
	}

	if ((this->family = strdup(family)) == NULL) {
		free(this->address);
		free(this->port);
		delete (this->addrlist);
		throw std::bad_alloc();
	}

	if ((this->pflist = new (std::nothrow) IghtStringVector(poller,
	    16)) == NULL) {
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
}

void
ConnectionState::connect_next(void)
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

		auto filedesc = ight_socket_create(storage.ss_family,
		    SOCK_STREAM, 0);
		if (filedesc == IGHT_SOCKET_INVALID)
			continue;

		error = bufferevent_setfd(this->bev, filedesc);
		if (error != 0) {
			(void) evutil_closesocket(filedesc);
			continue;
		}

		error = bufferevent_socket_connect(this->bev, (struct
		    sockaddr *) &storage, (int) total);
		if (error != 0) {
			(void) evutil_closesocket(filedesc);
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
	this->on_error_fn(IghtError(-2));
}

void
ConnectionState::handle_resolve(int result, char type,
		std::vector<std::string> results) {

	const char *_family;
	int error;

	ight_info("handle_resolve - enter");

	if (!connecting)
		abort();

	if (result != DNS_ERR_NONE)
		goto finally;

	switch (type) {
	case DNS_IPv4_A:
		ight_info("handle_resolve - IPv4");
		_family = "PF_INET";
		break;
	case DNS_IPv6_AAAA:
		ight_info("handle_resolve - IPv6");
		_family = "PF_INET6";
		break;
	default:
		abort();
	}

	for (auto& address : results) {
		ight_info("handle_resolve - address %s", address.c_str());
		error = addrlist->append(address.c_str());
		if (error != 0) {
			ight_warn("handle_resolve - cannot append");
			continue;
		}
		ight_info("handle_resolve - family %s", _family);
		error = pflist->append(_family);
		if (error != 0) {
			ight_warn("handle_resolve - cannot append");
			// Oops the two vectors are not in sync anymore now
			connecting = 0;
			on_error_fn(IghtError(-3));
			return;
		}
	}

    finally:
	if (must_resolve_ipv6) {
		must_resolve_ipv6 = 0;
		bool ok = resolve_internal(DNS_IPv6_AAAA);
		if (ok)
			return;
		/* FALLTHROUGH */
	}
	if (must_resolve_ipv4) {
		must_resolve_ipv4 = 0;
		bool ok = resolve_internal(DNS_IPv4_A);
		if (ok)
			return;
		/* FALLTHROUGH */
	}
	connect_next();
}

bool ConnectionState::resolve_internal(char type) {

    std::string query;

    if (type == DNS_IPv6_AAAA) {
        query = "AAAA";
    } else if (type == DNS_IPv4_A) {
        query = "A";
    } else {
        return false;
    }

    try {
        dns_request = dns::Request(query, address, [this, type](
                dns::Response&& resp) {
            handle_resolve(resp.get_evdns_status(), type,
                    resp.get_results());
        });
    } catch (...) {
        return false;  /* TODO: save the error */
    }

    return true;
}

void
ConnectionState::resolve(ConnectionState *self)
{
	struct sockaddr_storage storage;
	int result;

	if (!self->connecting)
		abort();

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
			self->on_error_fn(IghtError(-4));
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
			self->on_error_fn(IghtError(-4));
			return;
		}
		self->connect_next();
		return;
	}

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
		self->on_error_fn(IghtError(-5));
		return;
	}

	bool ok = false;

	if (self->must_resolve_ipv4) {
		self->must_resolve_ipv4 = 0;
		ok = self->resolve_internal(DNS_IPv4_A);
	} else {
		self->must_resolve_ipv6 = 0;
		ok = self->resolve_internal(DNS_IPv6_AAAA);
	}
	if (!ok) {
		self->connecting = 0;
		self->on_error_fn(IghtError(-6));
		return;
	}

	// Arrange for the next resolve operation that we will need
	if (strcmp(self->family, "PF_UNSPEC") == 0)
		self->must_resolve_ipv6 = 1;
	else if (strcmp(self->family, "PF_UNSPEC6") == 0)
		self->must_resolve_ipv4 = 1;
}

void
ConnectionState::close(void)
{
	this->bev.close();
	this->dns_request.cancel();
}
