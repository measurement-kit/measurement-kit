/*
 * LibNeubot interface - Public domain.
 */

/* Classes: */

struct NeubotEchoServer;
struct NeubotEvent;
struct NeubotPollable;
struct NeubotPoller;

/* Callbacks: */

typedef void (*NeubotPoller_callback)(void *);
typedef void (*NeubotPollable_callback)(struct NeubotPollable *);

/* NeubotEchoServer API: */

struct NeubotEchoServer * NeubotEchoServer_construct(struct NeubotPoller *, int, const char *, const char *);

/* NeubotEvent API: */

void NeubotEvent_cancel(struct NeubotEvent *);

/* NeubotPollable API: */

struct NeubotPollable * NeubotPollable_construct(struct NeubotPoller *, NeubotPollable_callback, NeubotPollable_callback, NeubotPollable_callback, void *);

void * NeubotPollable_opaque(struct NeubotPollable *);

struct NeubotPoller * NeubotPollable_poller(struct NeubotPollable *);

int NeubotPollable_attach(struct NeubotPollable *, long long);

void NeubotPollable_detach(struct NeubotPollable *);

long long NeubotPollable_fileno(struct NeubotPollable *);

int NeubotPollable_set_readable(struct NeubotPollable *);

int NeubotPollable_unset_readable(struct NeubotPollable *);

int NeubotPollable_set_writable(struct NeubotPollable *);

int NeubotPollable_unset_writable(struct NeubotPollable *);

void NeubotPollable_set_timeout(struct NeubotPollable *, double);

void NeubotPollable_clear_timeout(struct NeubotPollable *);

void NeubotPollable_close(struct NeubotPollable *);

/* NeubotPoller API: */

struct NeubotPoller * NeubotPoller_construct(void);

int NeubotPoller_sched(struct NeubotPoller *, double, NeubotPoller_callback, void *);

struct NeubotEvent * NeubotPoller_defer_read(struct NeubotPoller *, long long, NeubotPoller_callback, NeubotPoller_callback, void *, double);

struct NeubotEvent * NeubotPoller_defer_write(struct NeubotPoller *, long long, NeubotPoller_callback, NeubotPoller_callback, void *, double);

void NeubotPoller_loop(struct NeubotPoller *);

void NeubotPoller_break_loop(struct NeubotPoller *);

