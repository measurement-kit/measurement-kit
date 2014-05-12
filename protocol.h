/* libneubot/protocol.h */

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

#ifndef LIBNEUBOT_PROTOCOL_H
# define LIBNEUBOT_PROTOCOL_H
# ifdef __cplusplus

/*-
 * Protocol
 *   The Protocol is a virtual class that allows to implement the
 *   traits of a protocol on top of, e.g., a Connection.
 */

struct NeubotPoller;

typedef void (*neubot_slot_vo)(void *);  /* XXX Duplicate definition */

struct NeubotProtocol {
	virtual void on_connect(void) {
		// TODO: override
	}

	virtual void on_ssl(void) {
		// TODO: override
	}

	virtual void on_data(void) {
		// TODO: override
	}

	virtual void on_flush(void) {
		// TODO: override
	}

	virtual void on_eof(void) {
		// TODO: override
	}

	virtual void on_error(void) {
		// TODO: override
	}

	// Defined out-of-line to avoid -Wweak-vtables warning
	virtual NeubotPoller *get_poller(void);

	virtual ~NeubotProtocol(void) {
		// TODO: override
	}
};

# endif  /* __cplusplus */
#endif  /* LIBNEUBOT_PROTOCOL_H */
