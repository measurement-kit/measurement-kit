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

struct NeubotPollableState;
struct NeubotPoller;

struct NeubotPollable {
#ifndef SWIG
	NeubotPollableState *state;
#endif
	NeubotPollable(void);
	int attach(NeubotPoller *, long long);
	void detach(void);
	long long fileno(void);
	int set_readable(void);
	int set_writable(void);
	int unset_readable(void);
	int unset_writable(void);
	void set_timeout(double);
	void clear_timeout(void);
	virtual void handle_error(void);
	virtual void handle_read(void);
	virtual void handle_write(void);
	virtual ~NeubotPollable(void);
};
