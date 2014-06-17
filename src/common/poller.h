/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#ifndef LIBIGHT_POLLER_H
# define LIBIGHT_POLLER_H

#include <functional>

struct event_base;
struct evdns_base;
struct IghtPoller;

IghtPoller *IghtPoller_construct(void);

event_base *IghtPoller_get_event_base(IghtPoller *);

evdns_base *IghtPoller_get_evdns_base(IghtPoller *);

int IghtPoller_sched(IghtPoller *, double, std::function<void(void)>);

void IghtPoller_loop(IghtPoller *);

void IghtPoller_break_loop(IghtPoller *);

#endif  // LIBIGHT_POLLER_H
