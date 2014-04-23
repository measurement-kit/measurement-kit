/*
 * Public domain, 2013 Simone Basso.
 */

struct timeval;

#ifdef __cplusplus
extern "C" {
#endif

void neubot_timeval_now(struct timeval *);

double neubot_time_now(void);

int neubot_listen(int, const char *, const char *);

void neubot_xfree(void *);

struct timeval *neubot_timeval_init(struct timeval *, double);

/* Note: at the moment in poller.c */
int neubot_socket_valid(long long);

#ifdef __cplusplus
}
#endif
