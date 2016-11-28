// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_DNS_SENDRECV_IMPL_HPP
#define SRC_LIBMEASUREMENT_KIT_DNS_SENDRECV_IMPL_HPP

#include <event2/util.h>

#include "../dns/getaddrinfo.hpp"
#include "../dns/sendrecv.hpp"
#include "../net/utils.hpp"

namespace mk {
namespace dns {

template <MK_MOCK_NAMESPACE(net, socket_create),
          MK_MOCK_ANONYMOUS_NAMESPACE(setsockopt),
          MK_MOCK(getaddrinfo_numeric_datagram),
          MK_MOCK_ANONYMOUS_NAMESPACE(sendto),
          MK_MOCK_ANONYMOUS_NAMESPACE(evutil_closesocket)>
ErrorOr<Var<socket_t>> send_impl(std::string nameserver, std::string port,
                                 std::string packet, Var<Logger> logger) {

    socket_t fd_ = net_socket_create(PF_INET, SOCK_DGRAM, 0, logger);
    logger->debug("dns: sockfd: %lld", (long long)fd_);
    if (fd_ == -1) {
        return SocketCreateError();
    }
    Var<socket_t> sock{new socket_t{fd_}, [](socket_t *fdesc) {
        if (fdesc != nullptr and *fdesc != -1) {
            /*
             * TODO: do we want to check whether `close()` fails?
             */
            (void)evutil_closesocket(*fdesc);  /* Use libevent's wrapper */
            delete fdesc;
        }
    }}; /* Guarantee cleanup */

    static const int on = 1;
    if (setsockopt(*sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) != 0) {
        return SetsockoptError();
    }
    logger->debug("dns: set REUSEADDR option");

    ErrorOr<Var<addrinfo>> maybe_ainfo =
        getaddrinfo_numeric_datagram(nameserver.c_str(), port.c_str(), logger);
    if (!maybe_ainfo) {
        return maybe_ainfo.as_error();
    }
    Var<addrinfo> ainfo = *maybe_ainfo;

    if (packet.size() > SSIZE_MAX) {    /* Defensive check */
        return IntegerOverflowError();
    }
    ssize_t sent_bytes = sendto(*sock, packet.data(), packet.size(), 0,
                                ainfo->ai_addr, ainfo->ai_addrlen);
    logger->debug("dns: sendto() result: %lld", (long long)sent_bytes);
    if (sent_bytes < 0) {
        return SendtoError();  /* TODO: map to errno (see #935) */
    }
    // Cast safe because the negative case is excluded above
    if ((size_t)sent_bytes != packet.size()) {
        return PacketTruncatedError();
    }

    return sock;
}

template <MK_MOCK_PREFIX(reactor, pollfd)>
void pollin_impl(Var<socket_t> sock, Callback<Error> callback,
                 Settings settings, Var<Reactor> reactor, Var<Logger> logger) {
    reactor_pollfd(*sock, MK_POLLIN,
                   [=](Error err, short flags) {
                       logger->debug("dns: pollfd() err=%s flags=%d",
                                     err.explain().c_str(), flags);
                       if (err) {
                           callback(err);
                           return;
                       }
                       if ((flags & MK_POLLIN) == 0) {    /* Defensive */
                           callback(UnexpectedPollFlagsError());
                           return;
                       }
                       callback(NoError());
                   },
                   settings.get("dns/timeout", 3.0), reactor);
}

template <MK_MOCK_ANONYMOUS_NAMESPACE(recv)>
ErrorOr<std::string> recv_impl(Var<socket_t> sock, Var<Logger> logger) {
    char buffer[8000]; /* "8k should be enough for everyone" (cit.) */
    /*
     * For now this is recv. Do we want recvfrom?
     */
    ssize_t count = recv(*sock, buffer, sizeof(buffer), 0);
    logger->debug("dns: recv result: %lld", (long long)count);
    if (count < 0) {
        return RecvError();
    }
    if (count == 0) {
        // At least, my understanding is that this should not happen
        return UnexpectedShortReadError();
    }
    /*
     * Note: here we're making a copy. I don't think we are _so_ concerned
     * with performance in this module to care about it. Also, the packet
     * is probably small (less than 1k) and so the copy is "cheap".
     */
    return std::string{buffer, (size_t)count};
}

template <MK_MOCK(send), MK_MOCK(pollin), MK_MOCK(recv)>
void sendrecv_impl(std::string nameserver, std::string port, std::string packet,
                   Callback<Error, std::string> callback, Settings settings,
                   Var<Reactor> reactor, Var<Logger> logger) {
    /*
     * This `call_soon()` here is to _schedule_ the execution of the
     * requested action and this make sure the callback is called _after_
     * the `sendrecv_impl()` function returns. I am really pissed off
     * when a callback is called before the function with which you did
     * registered the callback returns: it increases complexity.
     */
    reactor->call_soon([=]() {
        ErrorOr<Var<socket_t>> maybe_sock =
            send(nameserver, port, packet, logger);
        if (!maybe_sock) {
            callback(maybe_sock.as_error(), "");
            return;
        }
        pollin(*maybe_sock,
               [=](Error error) {
                   if (error) {
                       callback(error, "");
                       return;
                   }
                   ErrorOr<std::string> maybe_buff = recv(*maybe_sock, logger);
                   if (!maybe_buff) {
                       callback(maybe_buff.as_error(), "");
                       return;
                   }
                   callback(NoError(), *maybe_buff);
               },
               settings, reactor, logger);
    });
}

} // namespace dns
} // namespace mk
#endif
