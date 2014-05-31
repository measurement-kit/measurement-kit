/* libneubot/test/echo_client_api.c */

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

/*
 * Echo client that uses the _read() and _write() C API (which
 * implies a copy) to echo the data to the server.
 */

#include <stdlib.h>

#include "../src/net/connection.h"
#include "../src/common/log.h"
#include "../src/ight_wrappers.h"

struct EchoProtocol {
	struct IghtConnection *connection;
	struct IghtPoller *poller;
	struct IghtProtocol *protocol;
};

static void	EchoProtocol_on_connect(void *);
static void	EchoProtocol_on_data(void *);
static void	EchoProtocol_on_flush(void *);
static void	EchoProtocol_on_eof(void *);
static void	EchoProtocol_on_error(void *);
static void	EchoProtocol_destruct(struct EchoProtocol *);

static struct EchoProtocol *
EchoProtocol_connect(struct IghtPoller *p, const char *family,
    const char *address, const char *port, double timeo)
{
	struct EchoProtocol *self;
	int result;

	self = calloc(1, sizeof (*self));
	if (self == NULL)
		return (NULL);

	self->protocol = IghtProtocol_construct(p, EchoProtocol_on_connect,
	    NULL, EchoProtocol_on_data, EchoProtocol_on_flush,
	    EchoProtocol_on_eof, EchoProtocol_on_error, self);
	if (self->protocol == NULL) {
		free (self);
		return (NULL);
	}

	self->poller = p;

	self->connection = IghtConnection_connect(self->protocol, family,
	    address, port);
	if (self->connection == NULL) {
		IghtProtocol_destruct(self->protocol);
		free(self);
		return (NULL);
	}

	result = IghtConnection_set_timeout(self->connection, timeo);
	if (result != 0) {
		IghtConnection_close(self->connection);
		IghtProtocol_destruct(self->protocol);
		free(self);
		return (NULL);
	}

	result = IghtConnection_enable_read(self->connection);
	if (result != 0) {
		IghtConnection_close(self->connection);
		IghtProtocol_destruct(self->protocol);
		free(self);
		return (NULL);
	}

	return (self);
}

static void
EchoProtocol_on_connect(void *opaque)
{
	ight_info("echo - connected");
	(void) opaque;
}

static void
EchoProtocol_on_data(void *opaque)
{
	char buffer[8000];
	int result;
	struct EchoProtocol *self;

	self = (struct EchoProtocol *) opaque;

	for (;;) {
		result = IghtConnection_read(self->connection,
		    buffer, sizeof (buffer));
		if (result < 0) {
			EchoProtocol_destruct(self);
			return;
		}
		if (result == 0) {
			ight_info("echo - exhausted input buffer");
			break;
		}
		result = IghtConnection_write(self->connection,
		    buffer, (size_t) result);
		if (result != 0) {
			EchoProtocol_destruct(self);
			return;
		}
	}
}

static void
EchoProtocol_on_flush(void *opaque)
{
	ight_info("echo - flushed");
	(void) opaque;
}

static void
EchoProtocol_on_eof(void *opaque)
{
	ight_info("echo - eof");
	EchoProtocol_destruct((struct EchoProtocol *) opaque);
}

static void
EchoProtocol_on_error(void *opaque)
{
	ight_info("echo - error");
	EchoProtocol_destruct((struct EchoProtocol *) opaque);
}

static void
EchoProtocol_destruct(struct EchoProtocol *self)
{
	ight_info("echo - destructor");
	IghtConnection_close(self->connection);
	IghtProtocol_destruct(self->protocol);
	free(self);
}

int
main(void)
{
	struct IghtPoller *poller;
	struct EchoProtocol *self;

	ight_info("echo - creating the poller...");

	poller = IghtPoller_construct();
	if (poller == NULL)
		exit(EXIT_FAILURE);

	ight_info("echo - creating the connection...");

	self = EchoProtocol_connect(poller, "PF_INET",
	    "127.0.0.1", "54321", 7.0);
	if (self == NULL)
		exit(EXIT_FAILURE);

	ight_info("echo - poller loop...");

	IghtPoller_loop(poller);

	ight_info("echo - exit");

	exit(EXIT_SUCCESS);
}
