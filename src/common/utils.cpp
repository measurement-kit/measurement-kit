// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/common/logger.hpp>
#include "src/common/utils.hpp"
#include "ext/strtonum.h"

#include <algorithm>
#include <deque>
#include <cstddef>
#include <iosfwd>
#include <string>

#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <event2/util.h>

namespace mk {

void timeval_now(timeval *tv) {
    if (gettimeofday(tv, nullptr) != 0) {
        throw std::runtime_error("gettimeofday()");
    }
}

double time_now(void) {
    timeval tv;
    timeval_now(&tv);
    double result = tv.tv_sec + tv.tv_usec / (double)1000000.0;
    return result;
}

void utc_time_now(struct tm *utc) {
    time_t tv;
    tv = time(nullptr);
    gmtime_r(&tv, utc);
}

std::string timestamp(const struct tm *t) {
    char result[30];
    std::string ts;
    if (strftime(result, sizeof(result), "%Y-%m-%d %H:%M:%S", t) == 0) {
        throw std::runtime_error("strftime()");
    };
    return std::string(result);
}

evutil_socket_t listen(int use_ipv6, const char *address, const char *port) {
    sockaddr_storage storage;
    socklen_t salen;
    const char *family;
    evutil_socket_t filedesc;
    int result;

    if (use_ipv6)
        family = "PF_INET6";
    else
        family = "PF_INET";

    result = storage_init(&storage, &salen, family, address, port);
    if (result == -1) return -1;

    filedesc = socket_create(storage.ss_family, SOCK_STREAM, 0);
    if (filedesc == MEASUREMENT_KIT_SOCKET_INVALID) return -1;

    result = socket_listen(filedesc, &storage, salen);
    if (result != 0) {
        (void)evutil_closesocket(filedesc);
        return -1;
    }

    return filedesc;
}

/* Many system's free() handle nullptr; is this needed? */
void xfree(void *ptr) {
    if (ptr != nullptr) free(ptr);
}

timeval *timeval_init(timeval *tv, double delta) {
    debug("utils:timeval_init - enter");
    if (delta < 0) {
        debug("utils:timeval_init - no init needed");
        return nullptr;
    }
    tv->tv_sec = (time_t)floor(delta);
    tv->tv_usec = (suseconds_t)((delta - floor(delta)) * 1000000);
    debug("utils:timeval_init - ok");
    return tv;
}

int storage_init(sockaddr_storage *storage, socklen_t *salen,
                 const char *family, const char *address, const char *port) {
    int _family;
    /* TODO: support also AF_INET, AF_INET6, ... */
    if (strcmp(family, "PF_INET") == 0) {
        _family = PF_INET;
    } else if (strcmp(family, "PF_INET6") == 0) {
        _family = PF_INET6;
    } else {
        warn("utils:storage_init: invalid family");
        return -1;
    }
    return storage_init(storage, salen, _family, address, port);
}

int storage_init(sockaddr_storage *storage, socklen_t *salen, int _family,
                 const char *address, const char *port) {
    const char *errstr;
    int _port = (int)measurement_kit_strtonum(port, 0, 65535, &errstr);
    if (errstr != nullptr) {
        warn("utils:storage_init: invalid port");
        return -1;
    }
    return storage_init(storage, salen, _family, address, _port);
}

int storage_init(sockaddr_storage *storage, socklen_t *salen, int _family,
                 const char *address, int _port) {
    int result;

    info("utils:storage_init - enter");

    if (_port < 0 || _port > 65535) {
        warn("utils:storage_init: invalid port");
        return -1;
    }

    memset(storage, 0, sizeof(*storage));
    switch (_family) {

    case PF_INET6: {
        sockaddr_in6 *sin6 = (sockaddr_in6 *)storage;
        sin6->sin6_family = AF_INET6;
        sin6->sin6_port = htons(_port);
        if (address != nullptr) {
            result = inet_pton(AF_INET6, address, &sin6->sin6_addr);
            if (result != 1) {
                warn("utils:storage_init: invalid addr");
                return -1;
            }
        } else {
            sin6->sin6_addr = in6addr_any;
        }
        *salen = sizeof(sockaddr_in6);
        break;
    }

    case PF_INET: {
        sockaddr_in *sin = (sockaddr_in *)storage;
        sin->sin_family = AF_INET;
        sin->sin_port = htons(_port);
        if (address != nullptr) {
            result = inet_pton(AF_INET, address, &sin->sin_addr);
            if (result != 1) {
                warn("utils:storage_init: invalid addr");
                return -1;
            }
        } else {
            sin->sin_addr.s_addr = INADDR_ANY;
        }
        *salen = sizeof(sockaddr_in);
        break;
    }

    default:
        throw std::runtime_error("invalid case");
    }

    info("utils:storage_init - ok");
    return 0;
}

evutil_socket_t socket_create(int domain, int type, int protocol) {
    evutil_socket_t filedesc;
    int result;

    info("utils:socket - enter");

    filedesc = socket(domain, type, protocol);
    if (filedesc == MEASUREMENT_KIT_SOCKET_INVALID) {
        warn("utils:socket: cannot create socket");
        return MEASUREMENT_KIT_SOCKET_INVALID;
    }

    result = evutil_make_socket_nonblocking(filedesc);
    if (result != 0) {
        warn("utils:socket: cannot make nonblocking");
        (void)evutil_closesocket(filedesc);
        return MEASUREMENT_KIT_SOCKET_INVALID;
    }

    info("utils:socket - ok");
    return filedesc;
}

int socket_connect(evutil_socket_t filedesc, sockaddr_storage *storage,
                   socklen_t salen) {
    int result;

    info("utils:socket_connect - enter");

    result = connect(filedesc, (sockaddr *)storage, salen);
    if (result != 0) {
#ifndef WIN32
        if (errno == EINPROGRESS)
#else
        if (WSAGetLastError() == WSA_EINPROGRESS) /* untested */
#endif
            goto looksgood;
        warn("utils:socket_connect - connect() failed");
        return -1;
    }

looksgood:
    info("utils:socket_connect - ok");
    return 0;
}

int socket_listen(evutil_socket_t filedesc, sockaddr_storage *storage,
                  socklen_t salen) {
    int result, activate;

    info("utils:socket_listen - enter");

    activate = 1;
    result = setsockopt(filedesc, SOL_SOCKET, SO_REUSEADDR, &activate,
                        sizeof(activate));
    if (result != 0) {
        warn("utils:socket_listen - setsockopt() failed");
        return -1;
    }

    result = bind(filedesc, (sockaddr *)storage, salen);
    if (result != 0) {
        warn("utils:socket_listen - bind() failed");
        return -1;
    }

    result = ::listen(filedesc, 10);
    if (result != 0) {
        warn("utils:socket_listen - listen() failed");
        return -1;
    }

    info("utils:socket_listen - ok");
    return 0;
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

std::string unreverse_ipv6(std::string s) {
    size_t i = 0, added = 0;
    std::deque<char> r;
    for (; i < s.size(); ++i) {
        if ((i % 2) == 0) {
            if (!isxdigit(s[i])) break;
            r.push_front(s[i]);
            if ((++added % 4) == 0 && added <= 28) r.push_front(':');
        } else {
            if (s[i] != '.') return "";
        }
    }
    if (s.substr(i) != "ip6.arpa" && s.substr(i) != "ip6.arpa.") return "";
    return std::string(r.begin(), r.end());
}

std::string unreverse_ipv4(std::string s) {
    std::deque<char> r, t;
    size_t i = 0, seen = 0;
    unsigned cur = 0;
    for (; i < s.size(); ++i) {
        if (s[i] == '.') {
            if (cur > 255) return "";
            if (seen++ > 0) r.push_front('.');
            r.insert(r.begin(), t.begin(), t.end());
            t.clear();
            cur = 0;
        } else if (isdigit(s[i])) {
            t.push_back(s[i]);
            char tmpstr[] = {s[i], '\0'};
            cur = cur * 10 + atoi(tmpstr);
        } else
            break;
    }
    if (s.substr(i) != "in-addr.arpa" && s.substr(i) != "in-addr.arpa.")
        return "";
    return std::string(r.begin(), r.end());
}

} // namespace mk
