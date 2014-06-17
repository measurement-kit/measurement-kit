/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#ifndef LIBIGHT_POLLER_H
# define LIBIGHT_POLLER_H

struct event_base;
struct evdns_base;
struct IghtPoller;

typedef void (*ight_hook_vo)(void *);

IghtPoller *IghtPoller_construct(void);

event_base *IghtPoller_event_base_(IghtPoller *);

evdns_base *IghtPoller_evdns_base_(IghtPoller *);

int IghtPoller_sched(IghtPoller *, double, ight_hook_vo, void *);

void IghtPoller_loop(IghtPoller *);

void IghtPoller_break_loop(IghtPoller *);

#endif  // LIBIGHT_POLLER_H
