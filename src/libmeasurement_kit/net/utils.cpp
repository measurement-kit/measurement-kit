// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/ext/http_parser.h"
#include "src/libmeasurement_kit/net/error.hpp"
#include "src/libmeasurement_kit/net/utils.hpp"

#include <cassert>
#include <cstring>
#include <deque>
#include <sstream>
#include <system_error>

#include <event2/util.h>

namespace mk {
namespace net {

static Error make_sockaddr_ipv4(std::string s, uint16_t p, sockaddr_storage *ss,
                                socklen_t *solen) noexcept {
    sockaddr_storage ss4 = {};
    sockaddr_in *sin4 = (sockaddr_in *)&ss4;
    if (inet_pton(AF_INET, s.c_str(), &sin4->sin_addr) != 1) {
        return ValueError();
    }
    sin4->sin_family = AF_INET;
    sin4->sin_port = htons(p);
    if (ss != nullptr) {
        *ss = ss4;
    }
    if (solen != nullptr) {
        *solen = sizeof(*sin4);
    }
    return NoError();
}

static Error make_sockaddr_ipv6(std::string s, uint16_t p, sockaddr_storage *ss,
                                socklen_t *solen) noexcept {
    sockaddr_storage ss6 = {};
    sockaddr_in6 *sin6 = (sockaddr_in6 *)&ss6;
    if (inet_pton(AF_INET6, s.c_str(), &sin6->sin6_addr) != 1) {
        return ValueError();
    }
    sin6->sin6_family = AF_INET6;
    sin6->sin6_port = htons(p);
    if (ss != nullptr) {
        *ss = ss6;
    }
    if (solen != nullptr) {
        *solen = sizeof(*sin6);
    }
    return NoError();
}

bool is_ipv4_addr(std::string s) {
    return make_sockaddr_ipv4(s, 80, nullptr, nullptr) == NoError();
}

bool is_ipv6_addr(std::string s) {
    return make_sockaddr_ipv6(s, 80, nullptr, nullptr) == NoError();
}

bool is_ip_addr(std::string s) {
    return is_ipv4_addr(s) || is_ipv6_addr(s);
}

static ErrorOr<Endpoint> parse_endpoint_internal(std::string s) {
    http_parser_url parser = {};
    http_parser_url_init(&parser);
    /*
     * Here the trick is that we ask the parser to parse the URL typically
     * passed to the CONNECT header, which is in the format we want.
     */
    if (http_parser_parse_url(s.data(), s.size(), 1, &parser) != 0) {
        return {ValueError(), {}};
    }
    assert(parser.field_set == ((1 << UF_HOST) | (1 << UF_PORT)));
    Endpoint epnt = {};
    epnt.hostname = s.substr(parser.field_data[UF_HOST].off,
                             parser.field_data[UF_HOST].len);
    epnt.port = parser.port;
    return {NoError(), epnt};
}

static std::string serialize_address_port(std::string a, uint16_t p) {
    std::stringstream ss;
    bool is_ipv6 = is_ipv6_addr(a);
    if (is_ipv6) ss << "[";
    ss << a;
    if (is_ipv6) ss << "]";
    ss << ":";
    ss << p;
    std::string s = ss.str();
    return s;
}

ErrorOr<Endpoint> parse_endpoint(std::string s, uint16_t default_port) {
    ErrorOr<Endpoint> maybe_epnt = parse_endpoint_internal(s);
    if (!!maybe_epnt) {
        return maybe_epnt;
    }
    /*
     * The CONNECT parser is quite strict. It fails if the port is not
     * present. After first failure, retry adding the default port.
     */
    return parse_endpoint_internal(serialize_address_port(s, default_port));
}

ErrorOr<Endpoint>
endpoint_from_sockaddr_storage(sockaddr_storage *ss) noexcept {
    // Code adapted from src/libmeasurement_kit/dns/getaddrinfo_async.hpp
    char abuf[128];
    void *aptr = nullptr;
    Endpoint epnt;
    if (ss->ss_family == AF_INET) {
        aptr = &((sockaddr_in *)ss)->sin_addr;
        epnt.port = ntohs(((sockaddr_in *)ss)->sin_port);
    } else if (ss->ss_family == AF_INET6) {
        aptr = &((sockaddr_in6 *)ss)->sin6_addr;
        epnt.port = ntohs(((sockaddr_in6 *)ss)->sin6_port);
    } else {
        return {ValueError("invalid_family"), {}};
    }
    if (inet_ntop(ss->ss_family, aptr, abuf, sizeof(abuf)) == nullptr) {
        return {GenericError("inet_ntop_failure"), {}};
    }
    epnt.hostname = abuf;
    return {NoError(), epnt};
}

std::string serialize_endpoint(Endpoint epnt) {
    return serialize_address_port(epnt.hostname, epnt.port);
}

int storage_init(sockaddr_storage *storage, socklen_t *salen,
                 const char *family, const char *address, const char *port,
                 SharedPtr<Logger> logger) {
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
    return storage_init(storage, salen, _family, address, port, logger);
}

int storage_init(sockaddr_storage *storage, socklen_t *salen, int _family,
                 const char *address, const char *port, SharedPtr<Logger> logger) {
    const char *errstr;
    int _port = (int)mkp_strtonum(port, 0, 65535, &errstr);
    if (errstr != nullptr) {
        warn("utils:storage_init: invalid port");
        return -1;
    }
    return storage_init(storage, salen, _family, address, _port, logger);
}

int storage_init(sockaddr_storage *storage, socklen_t *salen, int _family,
                 const char *address, int _port, SharedPtr<Logger> logger) {
    int result;

    if (_port < 0 || _port > 65535) {
        logger->warn("utils:storage_init: invalid port");
        return -1;
    }

    /*
     * TODO: merge this code with the above helpers.
     */

    memset(storage, 0, sizeof(*storage));
    switch (_family) {

    case PF_INET6: {
        sockaddr_in6 *sin6 = (sockaddr_in6 *)storage;
        sin6->sin6_family = AF_INET6;
        sin6->sin6_port = htons(_port);
        if (address != nullptr) {
            result = inet_pton(AF_INET6, address, &sin6->sin6_addr);
            if (result != 1) {
                logger->warn("utils:storage_init: invalid addr");
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
                logger->warn("utils:storage_init: invalid addr");
                return -1;
            }
        } else {
            sin->sin_addr.s_addr = INADDR_ANY;
        }
        *salen = sizeof(sockaddr_in);
        break;
    }

    default:
        logger->warn("utils:storage_init - invalid family");
        return -1;
    }

    return 0;
}

socket_t socket_create(int domain, int type, int protocol, SharedPtr<Logger> logger) {
    socket_t filedesc;
    int result;

    filedesc = socket(domain, type, protocol);
    if (filedesc == -1) {
        logger->warn("utils:socket: cannot create socket");
        return -1;
    }

    result = evutil_make_socket_nonblocking(filedesc);
    if (result != 0) {
        logger->warn("utils:socket: cannot make nonblocking");
        (void)evutil_closesocket(filedesc);
        return -1;
    }

    return filedesc;
}

std::string unreverse_ipv6(std::string s) {
    size_t i = 0, added = 0;
    std::deque<char> r;
    for (; i < s.size(); ++i) {
        if ((i % 2) == 0) {
            if (!isxdigit(s[i])) {
                break;
            }
            r.push_front(s[i]);
            if ((++added % 4) == 0 && added <= 28) {
                r.push_front(':');
            }
        } else {
            if (s[i] != '.') {
                return "";
            }
        }
    }
    if (s.substr(i) != "ip6.arpa" && s.substr(i) != "ip6.arpa.") {
        return "";
    }
    return std::string(r.begin(), r.end());
}

std::string unreverse_ipv4(std::string s) {
    std::deque<char> r, t;
    size_t i = 0, seen = 0;
    unsigned cur = 0;
    for (; i < s.size(); ++i) {
        if (s[i] == '.') {
            if (cur > 255) {
                return "";
            }
            if (seen++ > 0) {
                r.push_front('.');
            }
            r.insert(r.begin(), t.begin(), t.end());
            t.clear();
            cur = 0;
        } else if (isdigit(s[i])) {
            t.push_back(s[i]);
            char tmpstr[] = {s[i], '\0'};
            cur = cur * 10 + atoi(tmpstr);
        } else {
            break;
        }
    }
    if (s.substr(i) != "in-addr.arpa" && s.substr(i) != "in-addr.arpa.") {
        return "";
    }
    return std::string(r.begin(), r.end());
}

Error disable_nagle(socket_t sockfd) {
    static const int disable = 1;
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&disable,
                   sizeof (disable)) != 0) {
        return SocketError();
    }
    return NoError();
}

Error map_errno(int error_code) {
    if (error_code == 0) {
        return NoError();
    }
    /*
     * In most Unix systems they are the same error. For the few systems in
     * which they are not (and note I don't even know whether measurement-kit
     * does compile on these systems), force EAGAIN to be EWOULDBLOCK.
     *
     * (Yes, EWOULDBLOCK is a BSD-ism, but I like it more.)
     */
    if (error_code == EAGAIN) {
        error_code = EWOULDBLOCK;
        // FALLTHROUGH
    }
#define XX(_code_, _name_, _descr_)                                            \
    if (std::make_error_condition(std::errc::_descr_).value() == error_code) { \
        return _name_();                                                       \
    }
    MK_NET_ERRORS_XX
#undef XX
    return GenericError();
}

Error make_sockaddr(std::string s, uint16_t p,
        sockaddr_storage *ss, socklen_t *solen) noexcept {
    Error err = make_sockaddr_ipv4(s, p, ss, solen);
    if (err != NoError()) {
        err = make_sockaddr_ipv6(s, p, ss, solen);
    }
    return err;
}

} // namespace net
} // namespace mk
