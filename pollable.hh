/* libneubot/pollable.hh */

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
// You must include <event/event.h> before this file to get the
// definition of evutil_socket_t.
//

struct NeubotPoller;
struct event;

namespace Neubot {
    class Pollable {
        NeubotPoller *poller;
        double timeout;
        event *evread;
        event *evwrite;
        evutil_socket_t fileno;
        int setunset(const char *, unsigned, event *);
    protected:
        Pollable(void);
        int init(NeubotPoller *);
    public:
        static Pollable *construct(NeubotPoller *);
        int attach(long long);
        void detach(void);
        long long get_fileno(void);
        int set_readable(void);
        int set_writable(void);
        int unset_readable(void);
        int unset_writable(void);
        void set_timeout(double);
        void clear_timeout(void);
        virtual void handle_error(void);
        virtual void handle_read(void);
        virtual void handle_write(void);
        virtual ~Pollable(void);
    };
};
