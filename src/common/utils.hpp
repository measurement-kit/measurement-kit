// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef SRC_COMMON_UTILS_HPP
#define SRC_COMMON_UTILS_HPP

#include <event2/util.h>

#include <limits.h>
#include <stddef.h>
#include <unistd.h>

#include <string>
#include <iosfwd>

struct sockaddr_storage;
struct timeval;

namespace measurement_kit {

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
#define MEASUREMENT_KIT_SOCKET_INVALID -1

static inline int socket_valid_win32_(intptr_t filenum) {
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
     * This explains why libevent on Windows defines the socket to be
     * intptr_t rather than uintptr_t: because they use a negative
     * value, -1, to represent the invalid socket.
     *
     * For reference, wine's winsock.h defines in fact SOCKET as
     * UINT_PTR and INVALID_SOCKET as (~0):
     *
     *     http://goo.gl/Mo4ReU  (github.com/wine-mirror/wine)
     *
     * Note that the unsigned value ~0 is equal to -1 if converted to a
     * signed value of the same size (i.e., uintptr_t => intptr_t).
     *
     * To conclude, only -1 (equal to ~0) is invalid socket, hence
     * the check below:
     */
    return (filenum != MEASUREMENT_KIT_SOCKET_INVALID);
}

static inline int socket_valid_unix_(int filenum) {
    return (filenum >= 0 && filenum <= INT_MAX);
}

static inline int socket_valid(evutil_socket_t filenum) {
#ifdef WIN32
    return (socket_valid_win32_(filenum));
#else
    return (socket_valid_unix_(filenum));
#endif
}

static inline intptr_t socket_normalize_if_invalid_win32_(intptr_t filenum) {
    /*
     * Nothing do to; on Windows there is a single, canonical
     * representation of the invalid socket: ~0.
     */
    return (filenum);
}

static inline int socket_normalize_if_invalid_unix_(int filenum) {
    /*
     * This makes sense only in the Unix world in which all negative
     * integers different from -1 are non-canonical representations of
     * the invalid socket descriptor.
     */
    if (!socket_valid(filenum)) filenum = MEASUREMENT_KIT_SOCKET_INVALID;
    return (filenum);
}

static inline evutil_socket_t
socket_normalize_if_invalid(evutil_socket_t filenum) {
#ifdef WIN32
    return (socket_normalize_if_invalid_win32_(filenum));
#else
    return (socket_normalize_if_invalid_unix_(filenum));
#endif
}

/*
 * Other utility functions:
 */

void timeval_now(timeval *);

double time_now();

evutil_socket_t listen(int, const char *, const char *);

void xfree(void *);

timeval *timeval_init(timeval *, double);

int storage_init(sockaddr_storage *, socklen_t *, const char *, const char *,
                 const char *);

int storage_init(sockaddr_storage *, socklen_t *, int, const char *,
                 const char *);

int storage_init(sockaddr_storage *, socklen_t *, int, const char *, int);

evutil_socket_t socket_create(int, int, int);

int socket_connect(evutil_socket_t, sockaddr_storage *, socklen_t);

int socket_listen(evutil_socket_t, sockaddr_storage *, socklen_t);

std::string random_str(size_t length);

std::string random_str_uppercase(size_t length);

std::string unreverse_ipv6(std::string s);

std::string unreverse_ipv4(std::string s);

} // namespace measurement_kit
#endif
