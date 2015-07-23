// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <arpa/inet.h>
#include <netinet/in.h>

#include <algorithm>

#include <errno.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <event2/event.h>

#include "ext/strtonum.h"
#include <measurement_kit/common/log.hpp>
#include <measurement_kit/common/utils.hpp>

namespace measurement_kit {

void timeval_now(struct timeval *tv) {
    if (gettimeofday(tv, NULL) != 0)
        abort();
}

double time_now(void) {
    struct timeval tv;
    double result;

    timeval_now(&tv);
    result = tv.tv_sec + tv.tv_usec / (double)1000000.0;

    return (result);
}

evutil_socket_t listen(int use_ipv6, const char *address,
                            const char *port) {
    struct sockaddr_storage storage;
    socklen_t salen;
    const char *family;
    evutil_socket_t filedesc;
    int result;

    if (use_ipv6)
        family = "PF_INET6";
    else
        family = "PF_INET";

    result = storage_init(&storage, &salen, family, address, port);
    if (result == -1)
        return (-1);

    filedesc = socket_create(storage.ss_family, SOCK_STREAM, 0);
    if (filedesc == MEASUREMENT_KIT_SOCKET_INVALID)
        return (-1);

    result = socket_listen(filedesc, &storage, salen);
    if (result != 0) {
        (void)evutil_closesocket(filedesc);
        return (-1);
    }

    return (filedesc);
}

/* Many system's free() handle NULL; is this needed? */
void xfree(void *ptr) {
    if (ptr != NULL)
        free(ptr);
}

struct timeval *timeval_init(struct timeval *tv, double delta) {
    info("utils:timeval_init - enter");

    if (delta < 0) {
        info("utils:timeval_init - no init needed");
        return (NULL);
    }
    tv->tv_sec = (time_t)floor(delta);
    tv->tv_usec = (suseconds_t)((delta - floor(delta)) * 1000000);

    info("utils:timeval_init - ok");
    return (tv);
}

int storage_init(struct sockaddr_storage *storage, socklen_t *salen,
                      const char *family, const char *address,
                      const char *port) {
    int _family;

    /* TODO: support also AF_INET, AF_INET6, ... */
    if (strcmp(family, "PF_INET") == 0) {
        _family = PF_INET;
    } else if (strcmp(family, "PF_INET6") == 0) {
        _family = PF_INET6;
    } else {
        warn("utils:storage_init: invalid family");
        return (-1);
    }

    return storage_init(storage, salen, _family, address, port);
}

int storage_init(struct sockaddr_storage *storage, socklen_t *salen,
                      int _family, const char *address,
                      const char *port) {
    int _port;
    const char *errstr;

    _port = (int)measurement_kit_strtonum(port, 0, 65535, &errstr);
    if (errstr != NULL) {
        warn("utils:storage_init: invalid port");
        return (-1);
    }

    return storage_init(storage, salen, _family, address, _port);
}

int storage_init(struct sockaddr_storage *storage, socklen_t *salen,
                      int _family, const char *address, int _port) {
    int result;

    info("utils:storage_init - enter");

    if (_port < 0 || _port > 65535) {
        warn("utils:storage_init: invalid port");
        return (-1);
    }

    memset(storage, 0, sizeof(*storage));
    switch (_family) {

    case PF_INET6: {
        struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)storage;
        sin6->sin6_family = AF_INET6;
        sin6->sin6_port = htons(_port);
        if (address != NULL) {
            result = inet_pton(AF_INET6, address, &sin6->sin6_addr);
            if (result != 1) {
                warn("utils:storage_init: invalid addr");
                return (-1);
            }
        } else {
            sin6->sin6_addr = in6addr_any;
        }
        *salen = sizeof(struct sockaddr_in6);
        break;
    }

    case PF_INET: {
        struct sockaddr_in *sin = (struct sockaddr_in *)storage;
        sin->sin_family = AF_INET;
        sin->sin_port = htons(_port);
        if (address != NULL) {
            result = inet_pton(AF_INET, address, &sin->sin_addr);
            if (result != 1) {
                warn("utils:storage_init: invalid addr");
                return (-1);
            }
        } else {
            sin->sin_addr.s_addr = INADDR_ANY;
        }
        *salen = sizeof(struct sockaddr_in);
        break;
    }

    default:
        abort();
    }

    info("utils:storage_init - ok");
    return (0);
}

evutil_socket_t socket_create(int domain, int type, int protocol) {
    evutil_socket_t filedesc;
    int result;

    info("utils:socket - enter");

    filedesc = socket(domain, type, protocol);
    if (filedesc == MEASUREMENT_KIT_SOCKET_INVALID) {
        warn("utils:socket: cannot create socket");
        return (MEASUREMENT_KIT_SOCKET_INVALID);
    }

    result = evutil_make_socket_nonblocking(filedesc);
    if (result != 0) {
        warn("utils:socket: cannot make nonblocking");
        (void)evutil_closesocket(filedesc);
        return (MEASUREMENT_KIT_SOCKET_INVALID);
    }

    info("utils:socket - ok");
    return (filedesc);
}

int socket_connect(evutil_socket_t filedesc,
                        struct sockaddr_storage *storage, socklen_t salen) {
    int result;

    info("utils:socket_connect - enter");

    result = connect(filedesc, (struct sockaddr *)storage, salen);
    if (result != 0) {
#ifndef WIN32
        if (errno == EINPROGRESS)
#else
        if (WSAGetLastError() == WSA_EINPROGRESS) /* untested */
#endif
            goto looksgood;
        warn("utils:socket_connect - connect() failed");
        return (-1);
    }

looksgood:
    info("utils:socket_connect - ok");
    return (0);
}

int socket_listen(evutil_socket_t filedesc,
                       struct sockaddr_storage *storage, socklen_t salen) {
    int result, activate;

    info("utils:socket_listen - enter");

    activate = 1;
    result = setsockopt(filedesc, SOL_SOCKET, SO_REUSEADDR, &activate,
                        sizeof(activate));
    if (result != 0) {
        warn("utils:socket_listen - setsockopt() failed");
        return (-1);
    }

    result = bind(filedesc, (struct sockaddr *)storage, salen);
    if (result != 0) {
        warn("utils:socket_listen - bind() failed");
        return (-1);
    }

    result = ::listen(filedesc, 10);
    if (result != 0) {
        warn("utils:socket_listen - listen() failed");
        return (-1);
    }

    info("utils:socket_listen - ok");
    return (0);
}

// Stolen from:
// http://stackoverflow.com/questions/440133/how-do-i-create-a-random-alpha-numeric-string-in-c
std::string random_str(size_t length) {
    auto randchar = []() -> char {
        const char charset[] = "0123456789"
                               "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                               "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[rand() % max_index];
    };
    std::string str(length, 0);
    std::generate_n(str.begin(), length, randchar);
    return str;
}

std::string random_str_uppercase(size_t length) {
    auto randchar = []() -> char {
        const char charset[] = "0123456789"
                               "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[rand() % max_index];
    };
    std::string str(length, 0);
    std::generate_n(str.begin(), length, randchar);
    return str;
}

}
