/*-
 * Part of measurement-kit <https://measurement-kit.github.io/>.
 * Measurement-kit is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 * =========================================================================
 *
 * Portions Copyright (c) 2015, Adriano Faggiani, Enrico Gregori,
 * Luciano Lenzini, Valerio Luconi
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/// Android implementation of prober

// This is meant to run on Android but can run on all Linux systems
#ifdef __linux__

#include <measurement_kit/common/error.hpp>
#include <measurement_kit/common/logger.hpp>
#include "src/common/utils.hpp"
#include <measurement_kit/traceroute/android.hpp>
#include <measurement_kit/traceroute/error.hpp>
#include <measurement_kit/traceroute/interface.hpp>

#include <event2/event.h>

#include <arpa/inet.h>
#include <linux/errqueue.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/uio.h>

#include <functional>
#include <stdexcept>
#include <string>

#include <stdint.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

struct event_base;

namespace mk {
namespace traceroute {

AndroidProber::AndroidProber(bool a, int port, event_base *c, Var<Logger> l)
    : use_ipv4_(a), evbase_(c), port_(port), logger(l) {}

void AndroidProber::init() {

    if (sockfd_ >= 0 && evp_ != nullptr)
        return;
    else if (sockfd_ >= 0 || evp_ != nullptr)
        throw std::runtime_error("Objects not properly initialized.");

    sockaddr_storage ss;
    socklen_t sslen;
    int level_sock, opt_recverr, level_proto, opt_recvttl, family;
    const int val = 1;

    logger->debug("AndroidProber(%d, %d, %p) => %p", use_ipv4_, port_,
              (void *)evbase_, (void *)this);

    if (use_ipv4_) {
        level_sock = SOL_IP;
        opt_recverr = IP_RECVERR;
        level_proto = IPPROTO_IP;
        opt_recvttl = IP_RECVTTL;
        family = AF_INET;
    } else {
        level_sock = SOL_IPV6;
        opt_recverr = IPV6_RECVERR;
        level_proto = IPPROTO_IPV6;
        opt_recvttl = IPV6_RECVHOPLIMIT;
        family = AF_INET6;
    }

    sockfd_ = mk::socket_create(family, SOCK_DGRAM, 0);
    if (sockfd_ == -1) {
        cleanup();
        error_cb_(SocketCreateError());
        return;
    }

    if (setsockopt(sockfd_, level_sock, opt_recverr, &val, sizeof(val)) != 0 ||
        setsockopt(sockfd_, level_proto, opt_recvttl, &val, sizeof(val)) != 0 ||
        setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) != 0) {
        cleanup();
        error_cb_(SetsockoptError());
        return;
    }

    if (mk::storage_init(&ss, &sslen, family, NULL, port_) != 0) {
        cleanup();
        error_cb_(StorageInitError());
        return;
    }
    if (bind(sockfd_, (sockaddr *)&ss, sslen) != 0) {
        cleanup();
        error_cb_(BindError());
        return;
    }

    // Note: since here we use `this`, object cannot be copied/moved
    if ((evp_ = event_new(evbase_, sockfd_, EV_READ, event_callback, this)) ==
        NULL) {
        cleanup();
        error_cb_(EventNewError());
        return;
    }
}

void AndroidProber::send_probe(std::string addr, int port, int ttl,
                               std::string payload, double timeout) {

    int ipproto, ip_ttl, family;
    sockaddr_storage ss;
    socklen_t sslen;
    timeval tv;

    init();

    logger->debug("send_probe(%s, %d, %d, %lu)", addr.c_str(), port,
              ttl, payload.length());

    if (sockfd_ < 0) {
        error_cb_(SocketAlreadyClosedError()); // already close()d
        return;
    }
    // Note: until we figure out exactly how to deal with overlapped
    // probes enforce to traceroute hop by hop only
    if (probe_pending_) {
        error_cb_(ProbeAlreadyPendingError());
        return;
    }

    if (use_ipv4_) {
        ipproto = IPPROTO_IP;
        ip_ttl = IP_TTL;
        family = PF_INET;
    } else {
        ipproto = IPPROTO_IPV6;
        ip_ttl = IPV6_UNICAST_HOPS;
        family = PF_INET6;
    }

    if (setsockopt(sockfd_, ipproto, ip_ttl, &ttl, sizeof(ttl)) != 0) {
        error_cb_(SetsockoptError());
        return;
    }

    if (mk::storage_init(&ss, &sslen, family, addr.c_str(), port) != 0) {
        error_cb_(StorageInitError());
        return;
    }

    if (clock_gettime(CLOCK_MONOTONIC, &start_time_) != 0) {
        error_cb_(ClockGettimeError());
        return;
    }

    // Note: cast to ssize_t safe because payload length is bounded
    // We may want however to increase the maximum accepted length
    if (payload.length() > 512) {
        error_cb_(PayloadTooLongError());
        return;
    }

    if (sendto(sockfd_, payload.data(), payload.length(), 0, (sockaddr *)&ss,
               sslen) != (ssize_t)payload.length()) {
        mk::warn("sendto() failed: errno %d", errno);
        error_cb_(SendtoError());
        return;
    }

    if (event_add(evp_, mk::timeval_init(&tv, timeout)) != 0) {
        error_cb_(EventAddError());
        return;
    }

    probe_pending_ = true;
}

ProbeResult AndroidProber::on_socket_readable() {
    int expected_level, expected_type_recverr, expected_type_ttl;
    uint8_t expected_origin;
    sock_extended_err *socket_error;
    unsigned char buff[512];
    char controlbuff[512];
    ProbeResult r;
    msghdr msg;
    cmsghdr *cmsg;
    iovec iov;
    timespec arr_time;
    sockaddr_storage storage;
    socklen_t solen;

    logger->debug("on_socket_readable()");

    if (!probe_pending_) throw NoProbePendingError();
    probe_pending_ = false;

    r.is_ipv4 = use_ipv4_;

    if (clock_gettime(CLOCK_MONOTONIC, &arr_time) != 0)
        throw ClockGettimeError();
    r.rtt = calculate_rtt(arr_time, start_time_);
    logger->debug("rtt = %lf", r.rtt);

    memset(buff, 0, sizeof(buff));
    iov.iov_base = buff;
    iov.iov_len = sizeof(buff);
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    memset(controlbuff, 0, sizeof(controlbuff));
    msg.msg_control = controlbuff;
    msg.msg_controllen = sizeof(controlbuff);
    msg.msg_flags = 0;
    if ((r.recv_bytes = recvmsg(sockfd_, &msg, MSG_ERRQUEUE)) < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) { // Defensive
            logger->debug("it seems we received a valid reply packet back");
            memset(&storage, 0, sizeof(storage));
            solen = sizeof(storage);
            if ((r.recv_bytes = recvfrom(sockfd_, buff, sizeof(buff), 0,
                                         (sockaddr *)&storage, &solen)) < 0) {
                throw std::runtime_error("recv() failed");
            }
            logger->debug("recv_bytes = %lu", r.recv_bytes);
            r.valid_reply = true;
            logger->debug("valid_reply = %d", r.valid_reply);
            r.reply = std::string((const char *)buff, r.recv_bytes);
            logger->debug("reply = <%lu bytes>", r.reply.length());
            r.interface_ip = get_source_addr(use_ipv4_, &storage);
            logger->debug("interface_ip = %s", r.interface_ip.c_str());
            return r;
        }
        throw std::runtime_error("recvmsg() failed");
    }
    logger->debug("recv_bytes = %lu", r.recv_bytes);

    if (use_ipv4_) {
        expected_level = SOL_IP;
        expected_type_recverr = IP_RECVERR;
        expected_type_ttl = IP_TTL;
        expected_origin = SO_EE_ORIGIN_ICMP;
    } else {
        expected_level = IPPROTO_IPV6;
        expected_type_recverr = IPV6_RECVERR;
        expected_type_ttl = IPV6_HOPLIMIT;
        expected_origin = SO_EE_ORIGIN_ICMP6;
    }

    for (cmsg = CMSG_FIRSTHDR(&msg); (cmsg); cmsg = CMSG_NXTHDR(&msg, cmsg)) {

        if (cmsg->cmsg_level != expected_level) {
            throw std::runtime_error("Unexpected socket level");
        }
        if (cmsg->cmsg_type != expected_type_recverr &&
            cmsg->cmsg_type != expected_type_ttl) {
            mk::warn("Received unexpected cmsg_type: %d", cmsg->cmsg_type);
            continue;
        }
        if (cmsg->cmsg_type == expected_type_ttl) {
            r.ttl = get_ttl(CMSG_DATA(cmsg));
            logger->debug("ttl = %d", r.ttl);
            continue;
        }

        // Be robust to refactoring
        if (cmsg->cmsg_type != expected_type_recverr)
            throw std::runtime_error("Assertion Failed.");

        socket_error = (sock_extended_err *)CMSG_DATA(cmsg);
        if (socket_error->ee_origin != expected_origin) {
            mk::warn("Received unexpected ee_type: %d", cmsg->cmsg_type);
            continue;
        }
        r.icmp_type = socket_error->ee_type;
        logger->debug("icmp_type = %d", r.icmp_type);
        r.icmp_code = socket_error->ee_code;
        logger->debug("icmp_code = %d", r.icmp_code);
        r.interface_ip = get_source_addr(use_ipv4_, socket_error);
        logger->debug("interface_ip = %s", r.interface_ip.c_str());
    }

    return r;
}

std::string AndroidProber::get_source_addr(const sockaddr_in *sin) {
    char ip[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &sin->sin_addr, ip, INET_ADDRSTRLEN) == NULL)
        throw std::runtime_error("inet_ntop failed");
    return std::string(ip);
}

std::string AndroidProber::get_source_addr(const sockaddr_in6 *sin6) {
    char ip[INET6_ADDRSTRLEN];
    if (inet_ntop(AF_INET6, &sin6->sin6_addr, ip, INET6_ADDRSTRLEN) == NULL)
        throw std::runtime_error("inet_ntop failed");
    return std::string(ip);
}

std::string AndroidProber::get_source_addr(bool use_ipv4,
                                           const sockaddr_storage *ss) {
    if (use_ipv4) {
        return get_source_addr((sockaddr_in *)ss);
    }
    return get_source_addr((sockaddr_in6 *)ss);
}

std::string AndroidProber::get_source_addr(bool use_ipv4,
                                           sock_extended_err *se) {
    if (use_ipv4) {
        const sockaddr_in *sin = (const sockaddr_in *)SO_EE_OFFENDER(se);
        return get_source_addr(sin);
    } else {
        const sockaddr_in6 *sin6 = (const sockaddr_in6 *)SO_EE_OFFENDER(se);
        return get_source_addr(sin6);
    }
}

double AndroidProber::calculate_rtt(struct timespec end,
                                    struct timespec start) {
    const long NSEC_PER_SEC = 1000000000;
    const long MICROSEC_PER_SEC = 1000000;
    timespec temp;
    if ((end.tv_nsec - start.tv_nsec) < 0) {
        temp.tv_sec = end.tv_sec - start.tv_sec - 1;
        temp.tv_nsec = NSEC_PER_SEC + end.tv_nsec - start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec - start.tv_sec;
        temp.tv_nsec = end.tv_nsec - start.tv_nsec;
    }
    long tmp = NSEC_PER_SEC * temp.tv_sec;
    tmp += temp.tv_nsec;
    double rtt_ms = (double)tmp / MICROSEC_PER_SEC;
    if (rtt_ms < 0) rtt_ms = -1.0; // XXX
    return rtt_ms;
}

void AndroidProber::event_callback(int, short event, void *opaque) {
    AndroidProber *prober = static_cast<AndroidProber *>(opaque);

    prober->logger->debug("event_callback(_, %d, %p)", event, opaque);

    if ((event & EV_TIMEOUT) != 0) {
        prober->on_timeout();
        prober->timeout_cb_();

    } else if ((event & EV_READ) != 0) {
        ProbeResult result;
        try {
            result = prober->on_socket_readable();
        } catch (Error &error) {
            prober->error_cb_(error);
            return;
        }
        prober->result_cb_(result);

    } else
        throw std::runtime_error("Unexpected event error");
}

void AndroidProber::cleanup() {
    logger->debug("cleanup(): %p", (void *)this);
    if (sockfd_ >= 0) {
        ::close(sockfd_);
        sockfd_ = -1;
    }
    // Note: we don't own evbase_
    if (evp_ != nullptr) {
        event_free(evp_);
        evp_ = NULL;
    }
}

} // namespace traceroute
} // namespace mk

#endif // __linux__
