/*
 * Public domain, 2013 Simone Basso.
 */

struct sockaddr_storage;
struct timeval;

#ifdef __cplusplus
extern "C" {
#endif

void ight_timeval_now(struct timeval *);

double ight_time_now(void);

evutil_socket_t ight_listen(int, const char *, const char *);

void ight_xfree(void *);

struct timeval *ight_timeval_init(struct timeval *, double);

int ight_storage_init(struct sockaddr_storage *, socklen_t *,
    const char *, const char *, const char *);

evutil_socket_t ight_socket_create(int, int, int);

int ight_socket_connect(evutil_socket_t, struct sockaddr_storage *,
    socklen_t);

int ight_socket_listen(evutil_socket_t, struct sockaddr_storage *,
    socklen_t);

#ifdef __cplusplus
}
#endif
