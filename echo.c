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

#include <event.h>

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
		NeubotPollable_close(self->pollable);
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
		NeubotPollable_close(self->pollable);
		return;
	}

	if (EVBUFFER_LENGTH(self->buffer) == 0)
		NeubotPollable_unset_writable(self->pollable);

	if (self->seen_eof)
		NeubotPollable_close(self->pollable);
}

static void
Connection_close(void *opaque)
{
	struct Connection *self;

	/*
	 * Perhaps obvious, but: in this function we don't call
	 * NeubotPollable_close() because this function is invoked
	 * by NeubotPollable_close().
	 */

	self = (struct Connection *) opaque;

	if (self->fileno != -1)
		(void) close(self->fileno);
	if (self->buffer != NULL)
		evbuffer_free(self->buffer);

	free(self);
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

	conn->pollable = NeubotPollable_construct(self->poller,
	    Connection_read, Connection_write, Connection_close, conn);
	if (conn->pollable == NULL)
		goto cleanup;

	/* _____________________________________________________________
	 *
	 * WARNING! From this point on, we have a complete object that
	 * can be free()d by using NeubotPollable_close() and consequently
	 * with Connection_close(). To make sure that Connection_close()
	 * deals with incomplete fileno and buffer fields, below we properly
	 * and explicitly initialize fileno to -1 and buffer to NULL.
	 * _____________________________________________________________
	 */

	conn->fileno = -1;
	conn->buffer = NULL;

	conn->fileno = accept(self->fileno, NULL, NULL);
	if (conn->fileno == -1)
		goto cleanup;

	conn->buffer = evbuffer_new();
	if (conn->buffer == NULL)
		goto cleanup;

	result = NeubotPollable_attach(conn->pollable,
	    (long long) conn->fileno);
	if (result != 0)
		goto cleanup;

	result = NeubotPollable_set_readable(conn->pollable);
	if (result == 0)
		return;		/* success */

      cleanup:
	if (conn != NULL && conn->pollable != NULL)
		NeubotPollable_close(conn->pollable);
	else
		neubot_xfree(conn);
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
