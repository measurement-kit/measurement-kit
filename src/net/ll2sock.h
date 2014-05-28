/*
 * Public domain, 2013-2014 Simone Basso.
 */

/*
 * Macros and functions useful to convert `long long` integers
 * to sockets. We use `long long`, because it can represent both
 * a Unix socket (a `int`) and a Win32 socket (a uintptr_t).
 */

#ifndef LL2SOCK_H
# define LL2SOCK_H

/* To convert long long to sockets */
#ifdef WIN32
# define NEUBOT_SOCKET_MAX (INVALID_SOCKET - 1)
#else
# define NEUBOT_SOCKET_MAX INT_MAX
#endif
#if !(LLONG_MAX >= NEUBOT_SOCKET_MAX)
# error "LLONG_MAX must be larger than NEUBOT_SOCKET_MAX"
#endif

/* To mark sockets as invalid */
#ifdef WIN32
# define NEUBOT_SOCKET_INVALID INVALID_SOCKET
#else
# define NEUBOT_SOCKET_INVALID -1
#endif

static inline int
neubot_socket_valid(long long filenum)
{
	return (filenum >= 0 && filenum <= NEUBOT_SOCKET_MAX);
}

#endif  /* LL2SOCK_H */
