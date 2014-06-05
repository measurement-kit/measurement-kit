/* libneubot/test/echo_client_evbuf.cpp */

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
 * Echo client that uses the semi-internal evbuf-based API to echo
 * data to the server without performing any copy.
 */

#include <new>

#include <stdlib.h>

#include "../src/net/connection.h"
#include "../src/common/log.h"
#include "../src/ight_wrappers.h"
#include "../src/net/protocol.h"

#include <event2/buffer.h>

class EchoProtocol : public IghtProtocol {
	IghtConnection *connection;
	evbuffer *evbuf;
	IghtPoller *poller;

	EchoProtocol(void) {
		this->connection = NULL;
		this->evbuf = NULL;
		this->poller = NULL;
	}

    public:
	static EchoProtocol *connect(IghtPoller *p, const char *family,
	    const char *address, const char *port, double timeo) {

		EchoProtocol *self = new (std::nothrow) EchoProtocol();
		if (self == NULL)
			return (NULL);

		self->evbuf = evbuffer_new();
		if (self->evbuf == NULL) {
			delete self;
			return (NULL);
		}

		self->poller = p;

		self->connection = IghtConnection::connect(self, family,
		    address, port);
		if (self->connection == NULL) {
			delete self;
			return (NULL);
		}

		int result = self->connection->set_timeout(timeo);
		if (result != 0) {
			delete self;
			return (NULL);
		}

		result = self->connection->enable_read();
		if (result != 0) {
			delete self;
			return (NULL);
		}

		return (self);
	}

	virtual void on_connect(void) {
		ight_info("echo - connected");
	}

	virtual void on_data(void) {
		int result = this->connection->read_into_(this->evbuf);
		if (result != 0) {
			delete this;
			return;
		}
		result = this->connection->write_from_(this->evbuf);
		if (result != 0) {
			delete this;
			return;
		}
	}

	virtual void on_flush(void) {
		ight_info("echo - flushed");
	}

	virtual void on_eof(void) {
		ight_info("echo - eof");
		delete this;
	}

	virtual void on_error(void) {
		ight_info("echo - error");
		delete this;
	}

	// Implemented out-of-line to avoid the -Wweak-vtables warning
	virtual IghtPoller *get_poller(void);

	virtual ~EchoProtocol(void) {
		ight_info("echo - destructor");
		if (this->connection != NULL)
			this->connection->close();
		if (this->evbuf != NULL)
			evbuffer_free(this->evbuf);
	}
};

// Implemented here to avoid the -Wweak-vtables warning
IghtPoller *
EchoProtocol::get_poller(void)
{
	return (this->poller);
}

int
main(void)
{
	IghtPoller *poller;
	EchoProtocol *self;

	ight_info("echo - creating the poller...");

	poller = IghtPoller_construct();
	if (poller == NULL)
		exit(EXIT_FAILURE);

	ight_info("echo - creating the connection...");

	self = EchoProtocol::connect(poller, "PF_INET",
	    "127.0.0.1", "54321", 7.0);
	if (self == NULL)
		exit(EXIT_FAILURE);

	ight_info("echo - poller loop...");

	IghtPoller_loop(poller);

	ight_info("echo - exit");

	exit(EXIT_SUCCESS);
}
