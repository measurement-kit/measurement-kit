/*
 * Public domain, 2013-2014 Simone Basso.
 */

struct sockaddr_storage;
struct timeval;

#include <event2/util.h>

#include <limits.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Macros and functions useful to check whether sockets are valid. We
 * use the same definition of sockets as libevent, i.e., a `int` on Unix
 * and a `intptr_t` on Win32.
 */

/* Range in which a socket is valid */
#ifdef WIN32
# define IGHT_SOCKET_MAX (INVALID_SOCKET - 1)
#else
# define IGHT_SOCKET_MAX INT_MAX
#endif
#if !(LLONG_MAX >= IGHT_SOCKET_MAX)
# error "LLONG_MAX must be larger than IGHT_SOCKET_MAX"
#endif

/* To mark sockets as invalid */
#ifdef WIN32
# define IGHT_SOCKET_INVALID INVALID_SOCKET
#else
# define IGHT_SOCKET_INVALID -1
#endif

static inline int
ight_socket_valid(evutil_socket_t filenum)
{
	return (filenum >= 0 && filenum <= IGHT_SOCKET_MAX);
}

/*
 * Other utility functions:
 */

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
