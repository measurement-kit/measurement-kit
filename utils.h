/*
 * Public domain, 2013 Simone Basso.
 */

struct sockaddr_storage;
struct timeval;

#ifdef __cplusplus
extern "C" {
#endif

void neubot_timeval_now(struct timeval *);

double neubot_time_now(void);

evutil_socket_t neubot_listen(int, const char *, const char *);

void neubot_xfree(void *);

struct timeval *neubot_timeval_init(struct timeval *, double);

int neubot_storage_init(struct sockaddr_storage *, socklen_t *,
    const char *, const char *, const char *);

evutil_socket_t neubot_socket_create(int, int, int);

int neubot_socket_connect(evutil_socket_t, struct sockaddr_storage *,
    socklen_t);

int neubot_socket_listen(evutil_socket_t, struct sockaddr_storage *,
    socklen_t);

#ifdef __cplusplus
}
#endif
