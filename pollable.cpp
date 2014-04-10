/*-
 * Copyright (c) 2014
 *     Nexa Center for Internet & Society, Politecnico di Torino (DAUIN)
 *     and Alessandro Quaranta <alessandro.quaranta92@gmail.com>.
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
// Pollable
//

#include <sys/queue.h>

#include <stdlib.h>

#include <event2/event.h>
#include <event2/event_compat.h>
#include <event2/event_struct.h>

#include "log.h"
#include "utils.h"

#include "pollable.hh"

struct NeubotPollableState {
	NeubotPoller *poller;
	double timeout;
	event evread;
	event evwrite;
	evutil_socket_t fileno;
};

NeubotPollable::NeubotPollable(void)
{
	neubot_warn("pollable: NeubotPollable()");
	state = NULL;
}

static void
dispatch(evutil_socket_t filenum, short event, void *opaque)
{
	NeubotPollable *pollable;

	(void)filenum;

	pollable = (NeubotPollable *) opaque;

	if (event & EV_READ)
		pollable->handle_read();
	else if (event & EV_WRITE)
		pollable->handle_write();
	else
		abort();
}

int
NeubotPollable::attach(NeubotPoller *poller, long long fileno)
{
	//
	// Sanity
	//

	neubot_warn("pollable: attach()");

	if (state != NULL) {
		neubot_warn("pollable: already initialized");
		return (-1);
	}	

	if (poller == NULL || !neubot_socket_valid(fileno)) {
		neubot_warn("pollable: invalid argument");
		return (-1);
	}

	//
	// Init
	//

	state = (NeubotPollableState *) calloc(1, sizeof (*state));
	if (state == NULL) {
		neubot_warn("pollable: calloc() failed");
		goto fail;
	}

	state->fileno = fileno;
	state->poller = poller;
	state->timeout = -1.0;

	event_set(&state->evread, fileno, EV_READ | EV_PERSIST,
	    dispatch, this);

	event_set(&state->evwrite, fileno, EV_WRITE | EV_PERSIST,
	    dispatch, this);

	return (0);

	//
	// Handle failure
	//

fail:
	if (state != NULL)
		free(state);
	state = NULL;
	return (-1);
}

void
NeubotPollable::detach(void)
{
	if (state == NULL)
		return;

	neubot_warn("pollable: detach()");

	//
	// We don't close the file descriptor because the Neubot pollable
	// class does not close the file description as well
	//

	state->fileno = -1;
	event_del(&state->evread);
	event_del(&state->evwrite);
	free(state);
	state = NULL;
}

long long
NeubotPollable::fileno(void)
{
	if (state == NULL) {
		neubot_warn("pollable: not initialized");
		return (-1);
	}

	return (state->fileno);
}

int
NeubotPollable::set_readable(void)
{
	int result;

	neubot_warn("pollable: set_readable()");

	if (state == NULL) {
		neubot_warn("pollable: not initialized");
		return (-1);
	}

	result = event_add(&state->evread, NULL);
	if (result != 0) {
		neubot_warn("pollable: event_add() failed");
		return (-1);
	}

	return (0);
}

int
NeubotPollable::set_writable(void)
{
	int result;

	neubot_warn("pollable: set_writable()");

	if (state == NULL) {
		neubot_warn("pollable: not initialized");
		return (-1);
	}

	result = event_add(&state->evwrite, NULL);
	if (result != 0) {
		neubot_warn("pollable: event_add() failed");
		return (-1);
	}

	return (0);
}

int
NeubotPollable::unset_readable(void)
{
	int result;

	neubot_warn("pollable: unset_readable()");

	if (state == NULL) {
		neubot_warn("pollable: not initialized");
		return (-1);
	}

	result = event_del(&state->evread);
	if (result != 0) {
		neubot_warn("pollable: event_del() failed");
		return (-1);
	}

	return (0);
}

int
NeubotPollable::unset_writable(void)
{
	int result;

	neubot_warn("pollable: unset_writable()");

	if (state == NULL) {
		neubot_warn("pollable: not initialized");
		return (-1);
	}

	result = event_del(&state->evwrite);
	if (result != 0) {
		neubot_warn("pollable: event_del() failed");
		return (-1);
	}

	return (0);
}

void
NeubotPollable::set_timeout(double timeout)
{
	/* TODO: implement */

	if (state == NULL) {
		neubot_warn("pollable: not initialized");
		return;
	}

	state->timeout = timeout;
}

void
NeubotPollable::clear_timeout(void)
{
	if (state == NULL) {
		neubot_warn("pollable: not initialized");
		return;
	}

	state->timeout = -1;
}

void
NeubotPollable::handle_read(void)
{
	// TODO: override
}

void
NeubotPollable::handle_write(void)
{
	// TODO: override
}

void
NeubotPollable::handle_error(void)
{
	// TODO: override
}

NeubotPollable::~NeubotPollable(void)
{
	neubot_warn("pollable: ~NeubotPollable()");
	this->detach();
}
