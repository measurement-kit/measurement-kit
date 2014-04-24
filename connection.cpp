/* libneubot/connection.cpp */

/*-
 * Copyright (c) 2013-2014
 *     Nexa Center for Internet & Society, Politecnico di Torino (DAUIN)
 *     and Simone Basso <bassosimone@gmail.com>.
 *
 * This file is part of Neubot <http://www.neubot.org/>.
 *
 * Neubot is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Neubot is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Neubot.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <limits.h>
#include <new>
#include <stdlib.h>
#include <string.h>

#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/event.h>

#include "ll2sock.h"
#include "poller.h"
#include "protocol.h"
#include "utils.h"

#include "connection.h"

NeubotConnection::NeubotConnection(void)
{
	this->filedesc = NEUBOT_SOCKET_INVALID;
	this->bev = NULL;
	this->protocol = NULL;
	this->readbuf = NULL;
	this->closing = 0;
	this->connecting = 0;
	this->reading = 0;
}

NeubotConnection::~NeubotConnection(void)
{
	if (this->filedesc != NEUBOT_SOCKET_INVALID)
		(void) evutil_closesocket((evutil_socket_t) this->filedesc);

	if (this->bev != NULL)
		bufferevent_free(this->bev);

	// protocol: should already be dead

	if (this->readbuf != NULL)
		evbuffer_free(this->readbuf);

	// closing: nothing to be done
	// connecting: nothing to be done
	// reading: nothing to be done
}

void
NeubotConnection::handle_read(bufferevent *bev, void *opaque)
{
	NeubotConnection *self = (NeubotConnection *) opaque;
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
NeubotConnection::handle_write(bufferevent *bev, void *opaque)
{
	NeubotConnection *self = (NeubotConnection *) opaque;
	(void) bev;  // Suppress warning about unused variable
	self->protocol->on_flush();
}

void
NeubotConnection::handle_event(bufferevent *bev, short what, void *opaque)
{
	NeubotConnection *self = (NeubotConnection *) opaque;

	(void) bev;  // Suppress warning about unused variable

	if (self->connecting && self->closing) {
		delete (self);
		return;
	} else if (self->connecting)
		self->connecting = 0;

	if (what & BEV_EVENT_CONNECTED) {
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

	// TODO: also handle the timeout

	self->protocol->on_error();
}

NeubotConnection *
NeubotConnection::attach(NeubotProtocol *proto, long long filenum)
{
	event_base *evbase;
	NeubotPoller *poller;
	NeubotConnection *self;
	int result;

	if (proto == NULL)
		abort();
	poller = proto->get_poller();
	if (poller == NULL)
		abort();
	evbase = NeubotPoller_event_base_(poller);
	if (evbase == NULL)
		abort();

	if (!neubot_socket_valid(filenum))
		return (NULL);

	self = new (std::nothrow) NeubotConnection();
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

	bufferevent_setcb(self->bev, self->handle_read, self->handle_write,
	    self->handle_event, self);

	result = bufferevent_enable(self->bev, EV_READ);
	if (result != 0) {
		delete self;
		return (NULL);
	}

	// Only own the filedesc when we know there were no errors
	self->filedesc = filenum;
	return (self);
}

NeubotConnection *
NeubotConnection::connect(NeubotProtocol *proto, const char *family,
    const char *address, const char *port)
{
	event_base *evbase;
	NeubotPoller *poller;
	int result;
	NeubotConnection *self;
	struct sockaddr_storage storage;
	socklen_t total;

	if (proto == NULL || family == NULL || address == NULL || port == NULL)
		abort();
	poller = proto->get_poller();
	if (poller == NULL)
		abort();
	evbase = NeubotPoller_event_base_(poller);
	if (evbase == NULL)
		abort();

	result = neubot_storage_init(&storage, &total, family, address, port);
	if (result != 0)
		return (NULL);

	self = new (std::nothrow) NeubotConnection();
	if (self == NULL)
		return (NULL);

	self->filedesc = neubot_socket_create(storage.ss_family,
	    SOCK_STREAM, 0);
	if (self->filedesc == NEUBOT_SOCKET_INVALID) {
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

	bufferevent_setcb(self->bev, self->handle_read, self->handle_write,
	    self->handle_event, self);

	// Note: cannot enable EV_READ until the connection is made

	result = bufferevent_socket_connect(self->bev, (struct sockaddr *)
	    &storage, (int) total);
	if (result != 0) {
		delete self;
		return (NULL);
	}

	return (self);
}

NeubotProtocol *
NeubotConnection::get_protocol(void)
{
	return (this->protocol);
}

int
NeubotConnection::set_timeout(double timeout)
{
	struct timeval tv, *tvp;
	tvp = neubot_timeval_init(&tv, timeout);
	return (bufferevent_set_timeouts(this->bev, tvp, tvp));
}

int
NeubotConnection::clear_timeout(void)
{
	return (this->set_timeout(-1));
}

int
NeubotConnection::start_tls(unsigned server_side)
{
	(void) server_side;

	return (-1);  // TODO: implement
}

int
NeubotConnection::read(char *base, size_t count)
{
	if (base == NULL || count == 0 || count > INT_MAX)
		return (-1);

	return (evbuffer_remove(this->readbuf, base, count));
}

int
NeubotConnection::readline(char *base, size_t count)
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

	int error = evbuffer_drain(this->readbuf, eol_length);
	if (error != 0)
		return (-1);

	return (llen);
}

int
NeubotConnection::readn(char *base, size_t count)
{
	if (base == NULL || count == 0 || count > INT_MAX)
		return (-1);
	if (evbuffer_get_length(this->readbuf) < count)
		return (0);

	return (evbuffer_remove(this->readbuf, base, count));
}

int
NeubotConnection::discardn(size_t count)
{
	if (count == 0 || count > INT_MAX)
		return (-1);
	if (evbuffer_get_length(this->readbuf) < count)
		return (0);

	return (evbuffer_drain(this->readbuf, count));
}

int
NeubotConnection::write(const char *base, size_t count)
{
	if (base == NULL || count == 0)
		return (-1);

	return (bufferevent_write(this->bev, base, count));
}

int
NeubotConnection::puts(const char *str)
{
	if (str == NULL)
		return (-1);

	return (this->write(str, strlen(str)));
}

int
NeubotConnection::read_into_(evbuffer *destbuf)
{
	if (destbuf == NULL)
		return (-1);

	return (evbuffer_add_buffer(destbuf, this->readbuf));
}

int
NeubotConnection::write_from_(evbuffer *sourcebuf)
{
	if (sourcebuf == NULL)
		return (-1);

	return (bufferevent_write_buffer(this->bev, sourcebuf));
}

void
NeubotConnection::close(void)
{
	this->closing = 1;
	if (this->reading != 0 || this->connecting != 0)
		return;
	delete this;
}
