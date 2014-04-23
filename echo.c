/* libneubot/echo.c */

/*-
 * Copyright (c) 2013
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

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <event2/buffer.h>
#include <event2/buffer_compat.h>
#include <event2/event.h>

#include "log.h"
#include "neubot.h"
#include "utils.h"

struct Connection {
	struct NeubotPollable *pollable;
	struct evbuffer *buffer;
	int seen_eof;
	int fileno;
};

struct NeubotEchoServer {
	struct NeubotPollable *pollable;
	struct NeubotPoller *poller;
	int fileno;
};

#define MAXREAD 8000

/*
 * Connection implementation
 */

static void
Connection_close(struct Connection *self)
{
	struct NeubotPollable *pollable;

	pollable = self->pollable;

	(void) close(self->fileno);
	evbuffer_free(self->buffer);
	free(self);

	NeubotPollable_close(pollable);
}

static void
Connection_read(void *opaque)
{
	struct Connection *self;
	int result;

	self = (struct Connection *) opaque;
	result = evbuffer_read(self->buffer, self->fileno, MAXREAD);
	if (result <= 0) {
		if (EVBUFFER_LENGTH(self->buffer) > 0) {
			self->seen_eof = 1;
			NeubotPollable_unset_readable(self->pollable);
			return;
		}
		Connection_close(self);
		return;
	}

	NeubotPollable_set_writable(self->pollable);
}

static void
Connection_write(void *opaque)
{
	struct Connection *self;
	int result;

	self = (struct Connection *) opaque;
	result = evbuffer_write(self->buffer, self->fileno);
	if (result == -1) {
		Connection_close(self);
		return;
	}

	if (EVBUFFER_LENGTH(self->buffer) == 0)
		NeubotPollable_unset_writable(self->pollable);

	if (self->seen_eof)
		Connection_close(self);
}

static void
Connection_error(void *opaque)
{
	struct Connection *self;
	self = (struct Connection *) opaque;
	Connection_close(self);
}

static void
Connection_construct(void *opaque)
{
	struct Connection *conn;
	struct NeubotEchoServer *self;
	int result;

	self = (struct NeubotEchoServer *) opaque;

	conn = calloc(1, sizeof(*conn));
	if (conn == NULL)
		goto cleanup;

	conn->buffer = NULL;
	conn->fileno = -1;
	conn->pollable = NULL;

	conn->buffer = evbuffer_new();
	if (conn->buffer == NULL)
		goto cleanup;

	conn->fileno = accept(self->fileno, NULL, NULL);
	if (conn->fileno == -1)
		goto cleanup;

	conn->pollable = NeubotPollable_construct(self->poller,
	    Connection_read, Connection_write, Connection_error, conn);
	if (conn->pollable == NULL)
		goto cleanup;

	result = NeubotPollable_attach(conn->pollable,
	    (long long) conn->fileno);
	if (result != 0)
		goto cleanup;

	result = NeubotPollable_set_readable(conn->pollable);
	if (result == 0)
		return;		/* success */

      cleanup:
	if (conn != NULL && conn->buffer != NULL)
		evbuffer_free(conn->buffer);
	if (conn != NULL && conn->fileno != -1)
		(void)close(conn->fileno);
	if (conn != NULL && conn->pollable != NULL)
		NeubotPollable_close(conn->pollable);
	free(conn);
}

/*
 * NeubotEchoServer implementation
 */

struct NeubotEchoServer *
NeubotEchoServer_construct(struct NeubotPoller *poller, int use_ipv6,
    const char *address, const char *port)
{
	struct NeubotEchoServer *self;
	int result;

	self = calloc(1, sizeof(*self));
	if (self == NULL)
		return (NULL);

	self->fileno = neubot_listen(use_ipv6, address, port);
	if (self->fileno == -1)
		goto cleanup;

	self->poller = poller;

	/*
	 * NOTE to self: the cleanup strategy must change if you add a
	 * a destructor function, because, in such case, NeubotPollable
	 * close() will also very likely free() self.
	 */
	self->pollable = NeubotPollable_construct(self->poller,
	    Connection_construct, NULL, NULL, self);
	if (self->pollable == NULL)
		goto cleanup;

	result = NeubotPollable_attach(self->pollable,
	    (long long) self->fileno);
	if (result != 0)
		goto cleanup;

	result = NeubotPollable_set_readable(self->pollable);
	if (result != 0)
		goto cleanup;

	return (self);

      cleanup:
	if (self->pollable != NULL)
		NeubotPollable_close(self->pollable);
	if (self->fileno != -1)
		(void) close(self->fileno);
	free(self);
	return (NULL);
}
