/*-
 * Copyright (c) 2015, Adriano Faggiani, Enrico Gregori, Luciano Lenzini,
 * Valerio Luconi
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

//
// This file contains the primitives needed to run a traceroute
// on Android platforms (and other Linux-based platforms).
//

// We use this on Android but we also compile this for Linux systems
#ifdef __linux__

#include <ight/portolan/traceroute_android.hpp>
#include <ight/common/log.hpp>
#include <ight/common/utils.hpp>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>

#include <netinet/in.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <netinet/ip_icmp.h>

#include <linux/errqueue.h>
#ifdef __ANDROID__
#include <linux/icmp.h>  // Needed to pull error code definitions
#endif

#include <arpa/inet.h>

#include <assert.h>
#include <time.h>
#include <unistd.h>

using namespace ight::portolan::traceroute_android;

struct ProbeResultMapping {
    unsigned char type;
    unsigned char code;
    Meaning meaning;
};

ProbeResultMapping MAPPINGv4[] = {
    {ICMP_TIME_EXCEEDED, ICMP_EXC_TTL, Meaning::TTL_EXCEEDED},
    {ICMP_DEST_UNREACH, ICMP_PORT_UNREACH, Meaning::DEST_REACHED},
    {ICMP_DEST_UNREACH, ICMP_PROT_UNREACH, Meaning::PROTO_NOT_IMPL},
    {ICMP_DEST_UNREACH, ICMP_NET_UNREACH, Meaning::NO_ROUTE_TO_HOST},
    {ICMP_DEST_UNREACH, ICMP_HOST_UNREACH, Meaning::ADDRESS_UNREACH},
    {255, 255, Meaning::OTHER},
};

ProbeResultMapping MAPPINGv6[] = {
    {ICMP6_TIME_EXCEEDED, ICMP6_TIME_EXCEED_TRANSIT, Meaning::TTL_EXCEEDED},
    {ICMP6_DST_UNREACH, ICMP6_DST_UNREACH_NOPORT, Meaning::DEST_REACHED},
    {ICMP6_DST_UNREACH, ICMP6_DST_UNREACH_NOROUTE, Meaning::NO_ROUTE_TO_HOST},
    {ICMP6_DST_UNREACH, ICMP6_DST_UNREACH_ADDR, Meaning::ADDRESS_UNREACH},
    {ICMP6_DST_UNREACH, ICMP6_DST_UNREACH_ADMIN, Meaning::ADMIN_FILTER},
    {255, 255, Meaning::OTHER},
};

Meaning ProbeResult::get_meaning() {
    for (auto m = is_ipv4 ? &MAPPINGv4[0] : &MAPPINGv6[0];
         m->meaning != Meaning::OTHER; ++m) {
        if (m->type == icmp_type && m->code == icmp_code) {
            return m->meaning;
        }
    }
    return Meaning::OTHER;
}

std::string Prober::get_source_addr(bool use_ipv4, sock_extended_err *se) {
    // TODO: I'm not annoyed by this function, if you are feel free to refactor
    if (use_ipv4) {
        char ip[INET_ADDRSTRLEN];
        const sockaddr_in *sin = (const sockaddr_in *)SO_EE_OFFENDER(se);
        if (inet_ntop(AF_INET, &sin->sin_addr, ip, INET_ADDRSTRLEN) == NULL)
            throw std::runtime_error("inet_ntop failed");
        return std::string(ip);
    } else {
        char ip[INET6_ADDRSTRLEN];
        const sockaddr_in6 *sin6 = (const sockaddr_in6 *)SO_EE_OFFENDER(se);
        if (inet_ntop(AF_INET6, &sin6->sin6_addr, ip, INET6_ADDRSTRLEN) == NULL)
            throw std::runtime_error("inet_ntop failed");
        return std::string(ip);
    }
}

double Prober::calculate_rtt(struct timespec end, struct timespec start) {
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
    if (rtt_ms < 0)
        rtt_ms = -1.0; // XXX
    return rtt_ms;
}

void Prober::event_callback(int, short event, void *opaque) {
    Prober *prober = static_cast<Prober *>(opaque);

    if ((event & EV_TIMEOUT) != 0) {
        prober->on_timeout_();
        prober->timeout_cb();

    } else if ((event & EV_READ) != 0) {
        ProbeResult result;
        try {
            result = prober->on_socket_readable_();
        } catch (std::runtime_error error) {
            prober->error_cb(error);
            return;
        }
        prober->result_cb(result);

    } else
        prober->error_cb(std::runtime_error("Unexpected event error"));
}

void Prober::close() {
    if (sockfd != -1) {
        ::close(sockfd);
        sockfd = -1;
    }
    // Note: we don't own evbase
    if (evbase != nullptr && evp != nullptr) {
        event_free(evp);
        evp = NULL;
    }
    // Note: this is to clear possible self references
    result_cb = nullptr;
    error_cb = nullptr;
    timeout_cb = nullptr;
}

Prober::Prober(bool use_ipv4_, int port, double timeout, event_base *evbase_) {

    sockaddr_storage ss;
    socklen_t sslen;
    int level_sock, opt_recverr, level_proto, opt_recvttl, family;
    const int val = 1;

    use_ipv4 = use_ipv4_;

    if (use_ipv4) {
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

    sockfd = ight_socket_create(family, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        close();
        throw std::runtime_error("Cannot create socket");
    }

    if (setsockopt(sockfd, level_sock, opt_recverr, &val, sizeof(val)) != 0 ||
        setsockopt(sockfd, level_proto, opt_recvttl, &val, sizeof(val)) != 0 ||
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) != 0) {
        close();
        throw std::runtime_error("Cannot set socket options");
    }

    if (ight_storage_init(&ss, &sslen, family, NULL, port) != 0) {
        close();
        throw std::runtime_error("ight_storage_init() failed");
    }
    if (bind(sockfd, (sockaddr *)&ss, sslen) != 0) {
        close();
        throw std::runtime_error("bind() failed");
    }

    evbase = evbase_;
    if (evbase != NULL) {
        timeval tv;
        // Note: since here we use this, object cannot be copied/moved
        if ((evp = event_new(evbase, sockfd, EV_READ | EV_PERSIST,
                             event_callback, this)) == NULL) {
            close();
            throw std::runtime_error("event_new() failed");
        }
        if (event_add(evp, ight_timeval_init(&tv, timeout)) != 0) {
            close();
            throw std::runtime_error("event_add() failed");
        }
    }
}

void Prober::send_probe(std::string addr, int port, int ttl,
                        std::string payload) {
    int ipproto, ip_ttl, family;
    sockaddr_storage ss;
    socklen_t sslen;

    if (sockfd == -1)
        throw std::runtime_error("Programmer error");  // already close()d

    // Note: until we figure out exactly how to deal with overlapped
    // probes enforce to traceroute hop by hop only
    if (probe_pending)
        throw std::runtime_error("Another probe is pending");

    if (use_ipv4) {
        ipproto = IPPROTO_IP;
        ip_ttl = IP_TTL;
        family = PF_INET;
    } else {
        ipproto = IPPROTO_IPV6;
        ip_ttl = IPV6_UNICAST_HOPS;
        family = PF_INET6;
    }

    if (setsockopt(sockfd, ipproto, ip_ttl, &ttl, sizeof(ttl)) != 0)
        throw std::runtime_error("setsockopt() failed");

    if (ight_storage_init(&ss, &sslen, family, addr.c_str(), port) != 0)
        throw std::runtime_error("ight_storage_init() failed");

    if (clock_gettime(CLOCK_MONOTONIC, &start_time) != 0)
        throw std::runtime_error("clock_gettime() failed");

    // Note: cast to ssize_t safe because payload length is bounded
    // We may want however to increase the maximum accepted length
    if (payload.length() > 512)
        throw std::runtime_error("payload too large");
    if (sendto(sockfd, payload.data(), payload.length(), 0, (sockaddr *)&ss,
               sslen) != (ssize_t) payload.length()) {
        throw std::runtime_error("sendto() failed");
    }

    probe_pending = true;
}

ProbeResult Prober::on_socket_readable_() {
    int expected_level, expected_type_recverr, expected_type_ttl;
    uint8_t expected_origin;
    sock_extended_err *socket_error;
    unsigned char buff[512];
    char controlbuff[512];
    ProbeResult r;
    msghdr msg;
    cmsghdr *cmsg;
    iovec iov;

    if (!probe_pending)
        throw std::runtime_error("No probe is pending");
    probe_pending = false;

    r.is_ipv4 = use_ipv4;

    timespec arr_time;
    if (clock_gettime(CLOCK_MONOTONIC, &arr_time) != 0)
        throw std::runtime_error("clock_gettime() failed");
    r.rtt = calculate_rtt(arr_time, start_time);

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
    if ((r.recv_bytes = recvmsg(sockfd, &msg, MSG_ERRQUEUE)) < 0)
        throw std::runtime_error("recvmsg() failed");

    if (use_ipv4) {
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
            ight_warn("Received unexpected cmsg_type: %d", cmsg->cmsg_type);
            continue;
        }
        if (cmsg->cmsg_type == expected_type_ttl) {
            r.ttl = get_ttl(CMSG_DATA(cmsg));
            continue;
        }

        // Be robust to refactoring
        assert(cmsg->cmsg_type == expected_type_recverr);

        socket_error = (sock_extended_err *)CMSG_DATA(cmsg);
        if (socket_error->ee_origin != expected_origin) {
            ight_warn("Received unexpected ee_type: %d", cmsg->cmsg_type);
            continue;
        }
        r.icmp_type = socket_error->ee_type;
        r.icmp_code = socket_error->ee_code;
        r.interface_ip = get_source_addr(use_ipv4, socket_error);
    }

    return r;
}

#endif // __linux__
