/*
 * Public domain, 2013-2014 Simone Basso.
 */

/*
 * Macros and functions useful to check whether sockets are valid. We
 * use the same definition of sockets as libevent, i.e., a `int` on Unix
 * and a `intptr_t` on Win32.
 */

#ifndef IGHT_NET_LL2SOCK_H
# define IGHT_NET_LL2SOCK_H

#include <event2/util.h>

#include <limits.h>
#include <stdint.h>

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

#endif  /* IGHT_NET_LL2SOCK_H */
