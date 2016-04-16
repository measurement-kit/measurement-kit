// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <arpa/inet.h>
#include <event2/dns.h>
#include <measurement_kit/common.hpp>
#include <measurement_kit/net.hpp>
#include <new>
#include <stdlib.h>
#include "src/net/connection.hpp"

extern "C" {

static void handle_libevent_read(bufferevent *, void *opaque) {
    static_cast<mk::net::Connection *>(opaque)->handle_read_();
}

static void handle_libevent_write(bufferevent *, void *opaque) {
    static_cast<mk::net::Connection *>(opaque)->handle_write_();
}

static void handle_libevent_event(bufferevent *, short what, void *opaque) {
    static_cast<mk::net::Connection *>(opaque)->handle_event_(what);
}

} // extern "C"
namespace mk {
namespace net {

void Connection::handle_read_() {
    Buffer buff(bufferevent_get_input(bev));
    try {
        emit_data(buff);
    } catch (Error &error) {
        emit_error(error);
    }
}

void Connection::handle_write_() {
    try {
        emit_flush();
    } catch (Error &error) {
        emit_error(error);
    }
}

void Connection::handle_event_(short what) {

    if (what & BEV_EVENT_CONNECTED) {
        connecting = 0;
        emit_connect();
        return;
    }

    if (what & BEV_EVENT_EOF) {
        emit_error(EofError());
        return;
    }

    if (connecting) {
        logger->info("connection::handle_event - try connect next");
        connect_next();
        return;
    }

    if (what & BEV_EVENT_TIMEOUT) {
        emit_error(TimeoutError());
        return;
    }

    emit_error(SocketError());
}

Connection::Connection(const char *family, const char *address,
                       const char *port, Poller *plr, Logger *lp,
                       evutil_socket_t filenum)
    : Emitter(lp), poller(plr) {
    filenum = mk::socket_normalize_if_invalid(filenum);

    this->bev.make(poller->get_event_base(), filenum,
                   BEV_OPT_DEFER_CALLBACKS | BEV_OPT_CLOSE_ON_FREE);

    // closing: nothing to be done

    if (!mk::socket_valid(filenum)) this->connecting = 1;

    this->address = address;

    this->port = port;

    this->family = family;

    // must_resolve_ipv4: if connecting, set later by this->resolve()
    // must_resolve_ipv6: if connecting, set later by this->resolve()

    /*
     * The following makes this non copyable and non movable.
     */
    bufferevent_setcb(this->bev, handle_libevent_read, handle_libevent_write,
                      handle_libevent_event, this);

    if (!mk::socket_valid(filenum))
        start_connect = DelayedCall(0.0, [this]() { this->resolve(); },
                                    Libs::global(), poller->get_event_base());
}

Connection::Connection(bufferevent *buffev, Poller *poller, Logger *logger)
        : Emitter(logger), poller(poller) {
    this->bev.set_bufferevent(buffev);

    // The following makes this non copyable and non movable.
    bufferevent_setcb(this->bev, handle_libevent_read, handle_libevent_write,
                      handle_libevent_event, this);
}

void Connection::connect_next() {
    const char *address;
    int error;
    const char *family;
    struct sockaddr_storage storage;
    socklen_t total;

    logger->info("connect_next - enter");

    for (;;) {
        if (addrlist.empty()) {
            logger->warn("connect_next - no more available addrs");
            break;
        }
        auto pair = addrlist.front();
        addrlist.pop_front();
        family = pair.first.c_str();
        address = pair.second.c_str();

        logger->info("connect_next - %s %s", family, address);

        error = mk::storage_init(&storage, &total, family, address,
                                              this->port.c_str());
        if (error != 0) continue;

        auto filedesc =
            mk::socket_create(storage.ss_family, SOCK_STREAM, 0);
        if (filedesc == MEASUREMENT_KIT_SOCKET_INVALID) continue;

        error = bufferevent_setfd(this->bev, filedesc);
        if (error != 0) {
            (void)evutil_closesocket(filedesc);
            continue;
        }

        error = bufferevent_socket_connect(
            this->bev, (struct sockaddr *)&storage, (int)total);
        if (error != 0) {
            (void)evutil_closesocket(filedesc);
            error =
                bufferevent_setfd(this->bev, MEASUREMENT_KIT_SOCKET_INVALID);
            if (error != 0) {
                logger->warn("connect_next - internal error");
                break;
            }
            continue;
        }

        logger->info("connect_next - ok");
        return;
    }

    this->connecting = 0;
    this->emit_error(ConnectFailedError());
}

void Connection::handle_resolve(Error error,
                                std::vector<dns::Answer> answers) {

    const char *_family;

    logger->info("handle_resolve - enter");

    if (!connecting) abort();

    if (error) goto finally;

    for (auto answer: answers) {
        std::string address;
        if (answer.type == dns::QueryTypeId::A) {
            address = answer.ipv4;
            _family = "PF_INET";
        } else if (answer.type == dns::QueryTypeId::AAAA) {
            address = answer.ipv6;
            _family = "PF_INET6";
        } else {
             abort();
             break;
        }
        logger->info("handle_resolve - address %s", address.c_str());
        logger->info("handle_resolve - family %s", _family);
        addrlist.push_back(std::make_pair(_family, address.c_str()));
    }

finally:
    if (must_resolve_ipv6) {
        must_resolve_ipv6 = 0;
        bool ok = resolve_internal(DNS_IPv6_AAAA);
        if (ok) return;
        /* FALLTHROUGH */
    }
    if (must_resolve_ipv4) {
        must_resolve_ipv4 = 0;
        bool ok = resolve_internal(DNS_IPv4_A);
        if (ok) return;
        /* FALLTHROUGH */
    }
    connect_next();
}

bool Connection::resolve_internal(char type) {

    dns::QueryTypeId query;

    if (type == DNS_IPv6_AAAA) {
        query = dns::QueryTypeId::AAAA;
    } else if (type == DNS_IPv4_A) {
        query = dns::QueryTypeId::A;
    } else {
        return false;
    }

    dns::query(
        dns::QueryClassId::IN, query,
        address, [this](Error error, dns::Message message) {
            handle_resolve(error, message.answers);
        }, {}, poller);

    return true;
}

void Connection::resolve() {
    struct sockaddr_storage storage;
    int result;

    if (!connecting) abort();

    // If address is a valid IPv4 address, connect directly
    memset(&storage, 0, sizeof(storage));
    result = inet_pton(PF_INET, address.c_str(), &storage);
    if (result == 1) {
        logger->info("resolve - address %s", address.c_str());
        logger->info("resolve - family PF_INET");
        addrlist.push_back(std::make_pair("PF_INET", address));
        connect_next();
        return;
    }

    // If address is a valid IPv6 address, connect directly
    memset(&storage, 0, sizeof(storage));
    result = inet_pton(PF_INET6, address.c_str(), &storage);
    if (result == 1) {
        logger->info("resolve - address %s", address.c_str());
        logger->info("resolve - family PF_INET6");
        addrlist.push_back(std::make_pair("PF_INET6", address));
        connect_next();
        return;
    }

    // Note: PF_UNSPEC6 means that we try with IPv6 first
    if (family == "PF_INET")
        must_resolve_ipv4 = 1;
    else if (family == "PF_INET6")
        must_resolve_ipv6 = 1;
    else if (family == "PF_UNSPEC")
        must_resolve_ipv4 = 1;
    else if (family == "PF_UNSPEC6")
        must_resolve_ipv6 = 1;
    else
        throw std::runtime_error("invalid PF_xxx");

    bool ok = false;

    if (must_resolve_ipv4) {
        must_resolve_ipv4 = 0;
        ok = resolve_internal(DNS_IPv4_A);
    } else {
        must_resolve_ipv6 = 0;
        ok = resolve_internal(DNS_IPv6_AAAA);
    }
    if (!ok) {
        connecting = 0;
        emit_error(DnsGenericError());
        return;
    }

    // Arrange for the next resolve operation that we will need
    if (family == "PF_UNSPEC")
        must_resolve_ipv6 = 1;
    else if (family == "PF_UNSPEC6")
        must_resolve_ipv4 = 1;
}

void Connection::close() {
    this->bev.close();
}

} // namespace net
} // namespace mk
