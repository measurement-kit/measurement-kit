// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <arpa/inet.h>

#include <new>
#include <stdlib.h>

#include <event2/dns.h>

#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/net/connection.hpp>

using namespace measurement_kit::common;

namespace measurement_kit {
namespace net {

Connection::~Connection() {}

void Connection::handle_read(bufferevent *bev, void *opaque) {
    auto self = (Connection *)opaque;
    (void)bev; // Suppress warning about unused variable
    Buffer buff(bufferevent_get_input(self->bev));
    self->emit_data(buff);
}

void Connection::handle_write(bufferevent *bev, void *opaque) {
    auto self = (Connection *)opaque;
    (void)bev; // Suppress warning about unused variable
    self->emit_flush();
}

void Connection::handle_event(bufferevent *bev, short what, void *opaque) {
    auto self = (Connection *)opaque;

    (void)bev; // Suppress warning about unused variable

    if (what & BEV_EVENT_CONNECTED) {
        self->connecting = 0;
        self->emit_connect();
        return;
    }

    if (what & BEV_EVENT_EOF) {
        self->emit_error(EOFError());
        return;
    }

    if (self->connecting) {
        self->logger->info("connection::handle_event - try connect next");
        self->connect_next();
        return;
    }

    if (what & BEV_EVENT_TIMEOUT) {
        self->emit_error(TimeoutError());
        return;
    }

    self->emit_error(SocketError());
}

Connection::Connection(const char *family, const char *address,
                       const char *port, Poller *poller,
                       Logger *lp, evutil_socket_t filenum) : Dumb(lp) {
    auto evbase = poller->get_event_base();

    filenum = measurement_kit::socket_normalize_if_invalid(filenum);

    this->bev.make(evbase, filenum,
                   BEV_OPT_DEFER_CALLBACKS | BEV_OPT_CLOSE_ON_FREE);

    // closing: nothing to be done

    if (!measurement_kit::socket_valid(filenum))
        this->connecting = 1;

    this->address = address;

    this->port = port;

    this->family = family;

    // must_resolve_ipv4: if connecting, set later by this->resolve()
    // must_resolve_ipv6: if connecting, set later by this->resolve()

    /*
     * The following makes this non copyable and non movable.
     */
    bufferevent_setcb(this->bev, this->handle_read, this->handle_write,
                      this->handle_event, this);

    if (!measurement_kit::socket_valid(filenum))
        this->start_connect =
            std::make_shared<DelayedCall>(0.0, [this]() { this->resolve(); });
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

        error =
            measurement_kit::storage_init(&storage, &total, family, address, this->port.c_str());
        if (error != 0)
            continue;

        auto filedesc = measurement_kit::socket_create(storage.ss_family, SOCK_STREAM, 0);
        if (filedesc == MEASUREMENT_KIT_SOCKET_INVALID)
            continue;

        error = bufferevent_setfd(this->bev, filedesc);
        if (error != 0) {
            (void)evutil_closesocket(filedesc);
            continue;
        }

        error = bufferevent_socket_connect(
            this->bev, (struct sockaddr *)&storage, (int)total);
        if (error != 0) {
            (void)evutil_closesocket(filedesc);
            error = bufferevent_setfd(this->bev, MEASUREMENT_KIT_SOCKET_INVALID);
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

void Connection::handle_resolve(int result, char type,
                                     std::vector<std::string> results) {

    const char *_family;

    logger->info("handle_resolve - enter");

    if (!connecting)
        abort();

    if (result != DNS_ERR_NONE)
        goto finally;

    switch (type) {
    case DNS_IPv4_A:
        logger->info("handle_resolve - IPv4");
        _family = "PF_INET";
        break;
    case DNS_IPv6_AAAA:
        logger->info("handle_resolve - IPv6");
        _family = "PF_INET6";
        break;
    default:
        abort();
    }

    for (auto &address : results) {
        logger->info("handle_resolve - address %s", address.c_str());
        logger->info("handle_resolve - family %s", _family);
        addrlist.push_back(std::make_pair(_family, address));
    }

finally:
    if (must_resolve_ipv6) {
        must_resolve_ipv6 = 0;
        bool ok = resolve_internal(DNS_IPv6_AAAA);
        if (ok)
            return;
        /* FALLTHROUGH */
    }
    if (must_resolve_ipv4) {
        must_resolve_ipv4 = 0;
        bool ok = resolve_internal(DNS_IPv4_A);
        if (ok)
            return;
        /* FALLTHROUGH */
    }
    connect_next();
}

bool Connection::resolve_internal(char type) {

    std::string query;

    if (type == DNS_IPv6_AAAA) {
        query = "AAAA";
    } else if (type == DNS_IPv4_A) {
        query = "A";
    } else {
        return false;
    }

    try {
        dns_request = dns::Request(query, address,
                                   [this, type](dns::Response &&resp) {
            handle_resolve(resp.get_evdns_status(), type, resp.get_results());
        });
    } catch (...) {
        return false; /* TODO: save the error */
    }

    return true;
}

void Connection::resolve() {
    struct sockaddr_storage storage;
    int result;

    if (!connecting)
        abort();

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
        emit_error(DNSGenericError());
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
    this->dns_request.cancel();
}

}}
