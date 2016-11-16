// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include "../common/utils.hpp"
#include "../dns/query.hpp"

// XXX Those two files are to be included in aaa_base.hpp
#include <arpa/nameser.h>
#include <netdb.h>

#include <ares.h>
#include <cassert>

namespace mk {
namespace dns {
namespace cares {

/*
 * mk::dns::cares::query() - Run single query using c-ares.
 *
 * Implementation note: we use low level c-ares APIs for sending and
 * receiving messages but we completely bypass high level APIs for
 * managing the connection with DNS servers, because:
 *
 * 1. this API is meant for querying specific DNS servers at low
 *    level mainly for running ooni-probe measurements
 *
 * 2. the `ares_send()` function receives a callback that may either
 *    be called immediately or later on, and my understanding of ares
 *    code is that such callback is called immediately if there is
 *    an obvious error however it is not called later on unless you
 *    call `ares_process()` when some mechanism e.g. `select()` tells
 *    you that the socket has become readable. So, that could be an
 *    implementation of the code, but I prefer the more obvious
 *    one where we explicitly manage sockets, given what we say above.
 */
MK_MOCK_TEMPLATE((
    <MK_MOCK(call_soon), MK_MOCK(ares_create_query), MK_MOCK(socket_create),
     MK_MOCK(setsockopt), MK_MOCK(storage_init), MK_MOCK(sendto),
     MK_MOCK(reactor_pollfd), MK_MOCK(recv), MK_MOCK(ares_parse_a_reply)>))
void query(QueryClass dns_class, QueryType dns_type, std::string name,
           Callback<Error, Var<Message>> cb, Settings settings,
           Var<Reactor> reactor, Var<Logger> logger) {

#define SCHED_ERROR(e_)                                                        \
    do {                                                                       \
        call_soon([=]() { cb(e_, nullptr); }, reactor);                        \
    } while (0)

    logger->debug("cares: query for '%s'", name.c_str());

    int ares_code;

    // TODO: add error types for common errors and make sure they wrap
    // underlying errors coming from libcares

    std::string nameserver = settings.get("dns/nameserver", std::string{});
    if (nameserver == "") {
        // This engine does not work unless you pass it a nameserver
        SCHED_ERROR(GenericError());
        return;
    }
    logger->debug("cares: nameserver is '%s'", nameserver.c_str());

    double timeout = settings.get("dns/timeout", 3.0);
    logger->debug("cares: timeout is %f", timeout);

    if (dns_class != QueryClassId::IN) {
        // Note: this engine only works for the "IN" domain
        SCHED_ERROR(GenericError());
        return;
    }
    logger->debug("cares: class is IN");

    unsigned char *buf = nullptr;
    int buflen = 0;
    int ares_class = ns_c_in;
    int ares_type = 0;
    if (dns_type == QueryTypeId::A) {
        ares_type = ns_t_a;
        logger->debug("cares: type is A");

    } else {
        SCHED_ERROR(GenericError());
        return;
    }

    unsigned short query_id = 3; // FIXME: we should pass here a specific ID
    static const int rd = 1;     // We desire recursion, right?

    logger->debug("cares: query id is %u", query_id);
    logger->debug("cares: is recursion desired? %d", rd);

    ares_code = ares_create_query(name.c_str(), ares_class, ares_type, query_id,
                                  rd, &buf, &buflen, 0);
    logger->debug("cares: create_query result: %d", ares_code);
    if (ares_code != 0) {
        SCHED_ERROR(GenericError());
        return;
    }
    assert(buf != nullptr);
    assert(buflen > 0);

    // XXX: remember to free buffer

    mk::socket_t sockfd = socket_create(PF_INET, SOCK_DGRAM, 0);
    logger->debug("cares: sockfd: %lld", sockfd);
    if (sockfd == -1) {
        SCHED_ERROR(GenericError());
        return;
    }

    // XXX: remember to cleanup socket

    static const int val = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) != 0) {
        SCHED_ERROR(GenericError());
        return;
    }
    logger->debug("cares: set REUSEADDR option");

    // FIXME: why do I need to pass PF_INET here and cannot deduce this
    // thing directly from the passed address?

    sockaddr_storage ss;
    socklen_t sslen;
    if (storage_init(&ss, &sslen, PF_INET, nameserver.c_str(), 53) != 0) {
        SCHED_ERROR(GenericError());
        return;
    }
    logger->debug("cares: initialized sockaddr_storage");

    // FIXME: in theory I should make sure there's no `ssize_t` overflow here
    // XXX: do we need to make the socket nonblocking?

    ssize_t sent_bytes = sendto(sockfd, buf, buflen, 0, (sockaddr *)&ss, sslen);
    if (sent_bytes < 0 || (size_t)sent_bytes != buflen) {
        SCHED_ERROR(GenericError());
        return;
    }
    logger->debug("cares: sendto() result: %lld", sent_bytes);

    reactor_pollfd(
        sockfd, MK_POLLIN,
        [=](Error err, short flags) {
            logger->debug("cares: pollfd() err: %s", err.explain().c_str());
            logger->debug("cares: pollfd() flags: %d", flags);
            if (err) {
                cb(err, nullptr);
                return;
            }
            if ((flags & MK_POLLIN) == 0) {
                cb(GenericError(), nullptr);
                return;
            }

            // Question: is recv() enough or would recvfrom() be more desirable
            // because it allows us to see who replied us?

            unsigned char response[8192];  // 8k should be enough, right?
            ssize_t recv_bytes = recv(sockfd, response, sizeof(response), 0);
            logger->debug("cares: recv result: %lld", recv_bytes);
            if (recv_bytes < 0) {
                cb(GenericError(), nullptr);
                return;
            }

            if (recv_bytes < HFIXEDSZ) {
                cb(GenericError(), nullptr);
                return;
            }

            hostent *host = nullptr;
            ares_addrttl ttls;
            memset(&ttls, 0, sizeof(ttls));
            int nttls = 0;
            // FIXME FIXME FIXME
            auto ares_code =
                ares_parse_a_reply(response, recv_bytes, &host, &ttls, &nttls);
            logger->debug("cares: parse A reply: %d", ares_code);
            if (ares_code != 0) {
                cb(GenericError(), nullptr);
                return;
            }

            // BIG FIXME FROM HERE BELOW

            assert(nttls >= 0);
            mk::debug("parsing okay");
            mk::debug("host name: %d", host->h_length);
            for (char **p = host->h_addr_list; *p != nullptr; ++p) {
                in_addr addr;
                memcpy(&addr, *p, 4);
                mk::debug("host name: %s", inet_ntoa(addr));
            }
        },
        timeout, reactor);
}

} // namespace cares
} // namespace dns
} // namespace mk
