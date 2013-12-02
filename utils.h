/*
 * Public domain, 2013 Simone Basso.
 */

struct timeval;

void neubot_timeval_now(struct timeval *);

double neubot_time_now(void);

int neubot_listen(int, const char *, const char *);
