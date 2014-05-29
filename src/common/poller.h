/*
 * Public domain, 2013 Simone Basso.
 */

/* Methods that we only use internally: */

#ifdef __cplusplus
extern "C" {
#endif

struct event_base *IghtPoller_event_base_(struct IghtPoller *);
struct evdns_base *IghtPoller_evdns_base_(struct IghtPoller *);

#ifdef __cplusplus
}
#endif
