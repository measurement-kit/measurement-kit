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

//
// Libneubot C wrappers for C++
//

#include <new>

#include "neubot.h"
#include "log.h"

#include "pollable.hh"

struct __NeubotPollable : public NeubotPollable {

	neubot_slot_vo on_handle_error;
	neubot_slot_vo on_handle_read;
	neubot_slot_vo on_handle_write;
	void *opaque;

	__NeubotPollable(neubot_slot_vo on_read, neubot_slot_vo on_write,
	    neubot_slot_vo on_error, void *opaque) {
		neubot_warn("__pollable: __NeubotPollable()");
		this->on_handle_error = on_error;
		this->on_handle_read = on_read;
		this->on_handle_write = on_write;
		this->opaque = opaque;
	};

	virtual void handle_read(void) {
		neubot_warn("__pollable: handle_read()");
		this->on_handle_read(this->opaque);
	};

	virtual void handle_write(void) {
		neubot_warn("__pollable: handle_write()");
		this->on_handle_write(this->opaque);
	};

	virtual void handle_error(void) {
		neubot_warn("__pollable: handle_error()");
		this->on_handle_error(this->opaque);
	};

	virtual ~__NeubotPollable(void) {
		neubot_warn("__pollable: ~__NeubotPollable()");
	};
};

struct NeubotPollable *
NeubotPollable_construct(neubot_slot_vo handle_read,
    neubot_slot_vo handle_write, neubot_slot_vo handle_error,
    void *opaque)
{
	return (new (std::nothrow) __NeubotPollable(handle_read,
	    handle_write, handle_error, opaque));
}

/*
 * Note: attach() is separated from construct(), because in Neubot there
 * are cases in which we create a pollable and then we attach() and detach()
 * file descriptors to it (e.g., the Connector object does that).
 */

int
NeubotPollable_attach(NeubotPollable *self, NeubotPoller *poller,
    long long fileno)
{
	/*
	 * Note: `long long` simplifies the interaction with Java and
	 * shall be wide enough to hold evutil_socket_t, which is `int`
	 * on Unix and `uintptr_t` on Windows.
	 */
	return (self->attach(poller, fileno));
}

void
NeubotPollable_detach(NeubotPollable *self)
{
	self->detach();
}

long long
NeubotPollable_fileno(NeubotPollable *self)
{
	return (self->fileno());
}

int
NeubotPollable_set_readable(NeubotPollable *self)
{
	return (self->set_readable());
}

int
NeubotPollable_unset_readable(NeubotPollable *self)
{
	return (self->unset_readable());
}

int
NeubotPollable_set_writable(NeubotPollable *self)
{
	return (self->set_writable());
}

int
NeubotPollable_unset_writable(NeubotPollable *self)
{
	return (self->unset_writable());
}

void
NeubotPollable_set_timeout(NeubotPollable *self, double timeout)
{
	self->set_timeout(timeout);
}

void
NeubotPollable_clear_timeout(NeubotPollable *self)
{
	self->clear_timeout();
}

void
NeubotPollable_close(NeubotPollable *self)
{
	delete (self);
}
