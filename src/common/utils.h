/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 */

#ifndef LIBIGHT_COMMON_UTILS_H
# define LIBIGHT_COMMON_UTILS_H

struct sockaddr_storage;
struct timeval;

#include <event2/util.h>

#include <limits.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Macros and functions useful to check whether sockets are valid. We
 * use the same definition of sockets as libevent, i.e., a `int` on Unix
 * and a `intptr_t` on Win32.
 */

/*
 * To mark sockets as invalid. Strictly speaking on Windows it should
 * be INVALID_SOCKET, which however is an unsigned constant. Since
 * libevent uses inptr_t for sockets, here we use -1 to indicate that
 * a socket is not valid, to avoid compiler warnings.
 *
 * See also the much longer comment on this topic below.
 */
#define IGHT_SOCKET_INVALID -1

static inline int
ight_socket_valid_win32_(intptr_t filenum)
{
	/*
	 * On Windows there is only one value marking the socket as invalid,
	 * all the other values are to be considered valid sockets:
	 *
	 * "Windows Sockets handles have no restrictions, other than that
	 *  the value INVALID_SOCKET is not a valid socket. Socket handles may
	 *  take any value in the range 0 to INVALID_SOCKET–1".
	 *
	 *     http://goo.gl/FTesjR  (msdn.microsoft.com)
	 *
	 * This makes me wonder why libevent defines the socket to be
	 * intptr_t rather than uintptr_t. Probably because they use a
	 * signed value to represent the invalid socket?
	 *
	 * For reference, wine's winsock.h defines in fact SOCKET as
	 * UINT_PTR and INVALID_SOCKET as (~0):
	 *
	 *     http://goo.gl/Mo4ReU  (github.com/wine-mirror/wine)
	 *
	 * I assume that libevent uses intptr_t for convenience given
	 * the negative return value and, since a socket is indeed a
	 * uintptr_t, that negative values but IGHT_SOCKET_INVALID are
	 * to be treated as valid sockets as well.
	 */
	return (filenum != IGHT_SOCKET_INVALID);
}

static inline int
ight_socket_valid_unix_(int filenum)
{
	return (filenum >= 0 && filenum <= INT_MAX);
}

static inline int
ight_socket_valid(evutil_socket_t filenum)
{
#ifdef WIN32
	return (ight_socket_valid_win32_(filenum));
#else
	return (ight_socket_valid_unix_(filenum));
#endif
}

static inline intptr_t
ight_socket_normalize_if_invalid_win32_(intptr_t filenum)
{
	return (filenum);
}

static inline int
ight_socket_normalize_if_invalid_unix_(int filenum)
{
	/*
	 * This makes sense only in the Unix world in which a negative
	 * integer is not a valid socket descriptor (i.e., in which there
	 * are several non canonical invalid-socket values).
	 */
	if (!ight_socket_valid(filenum))
		filenum = IGHT_SOCKET_INVALID;
	return (filenum);
}

static inline evutil_socket_t
ight_socket_normalize_if_invalid(evutil_socket_t filenum)
{
#ifdef WIN32
	return (ight_socket_normalize_if_invalid_win32_(filenum));
#else
	return (ight_socket_normalize_if_invalid_unix_(filenum));
#endif
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

#endif  /* LIBIGHT_COMMON_UTILS_H */
