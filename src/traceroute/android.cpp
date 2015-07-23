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

#include <measurement_kit/common/log.hpp>
#include <measurement_kit/common/utils.hpp>
#include <measurement_kit/traceroute/android.hpp>

#include <arpa/inet.h>
#include <linux/errqueue.h>

#include <assert.h>
#include <string.h>
#include <unistd.h>

namespace measurement_kit {
namespace traceroute {

AndroidProber::AndroidProber(bool a, int port, event_base *c)
    : use_ipv4_(a), evbase_(c) {

  sockaddr_storage ss;
  socklen_t sslen;
  int level_sock, opt_recverr, level_proto, opt_recvttl, family;
  const int val = 1;

  measurement_kit::debug("AndroidProber(%d, %d, %p) => %p", use_ipv4_, port,
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

  sockfd_ = measurement_kit::socket_create(family, SOCK_DGRAM, 0);
  if (sockfd_ == -1) {
    cleanup();
    throw std::runtime_error("Cannot create socket");
  }

  if (setsockopt(sockfd_, level_sock, opt_recverr, &val, sizeof(val)) != 0 ||
      setsockopt(sockfd_, level_proto, opt_recvttl, &val, sizeof(val)) != 0 ||
      setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) != 0) {
    cleanup();
    throw std::runtime_error("Cannot set socket options");
  }

  if (measurement_kit::storage_init(&ss, &sslen, family, NULL, port) != 0) {
    cleanup();
    throw std::runtime_error("measurement_kit::storage_init() failed");
  }
  if (bind(sockfd_, (sockaddr *)&ss, sslen) != 0) {
    cleanup();
    throw std::runtime_error("bind() failed");
  }

  // Note: since here we use `this`, object cannot be copied/moved
  if ((evp_ = event_new(evbase_, sockfd_, EV_READ, event_callback, this)) ==
      NULL) {
    cleanup();
    throw std::runtime_error("event_new() failed");
  }
}

void AndroidProber::send_probe(std::string addr, int port, int ttl,
                               std::string payload, double timeout) {
  int ipproto, ip_ttl, family;
  sockaddr_storage ss;
  socklen_t sslen;
  timeval tv;

  measurement_kit::debug("send_probe(%s, %d, %d, %lu)", addr.c_str(), port, ttl,
             payload.length());

  if (sockfd_ < 0)
    throw std::runtime_error("Programmer error"); // already close()d

  // Note: until we figure out exactly how to deal with overlapped
  // probes enforce to traceroute hop by hop only
  if (probe_pending_)
    throw std::runtime_error("Another probe is pending");

  if (use_ipv4_) {
    ipproto = IPPROTO_IP;
    ip_ttl = IP_TTL;
    family = PF_INET;
  } else {
    ipproto = IPPROTO_IPV6;
    ip_ttl = IPV6_UNICAST_HOPS;
    family = PF_INET6;
  }

  if (setsockopt(sockfd_, ipproto, ip_ttl, &ttl, sizeof(ttl)) != 0)
    throw std::runtime_error("setsockopt() failed");

  if (measurement_kit::storage_init(&ss, &sslen, family, addr.c_str(), port) != 0)
    throw std::runtime_error("measurement_kit::storage_init() failed");

  if (clock_gettime(CLOCK_MONOTONIC, &start_time_) != 0)
    throw std::runtime_error("clock_gettime() failed");

  // Note: cast to ssize_t safe because payload length is bounded
  // We may want however to increase the maximum accepted length
  if (payload.length() > 512)
    throw std::runtime_error("payload too large");
  if (sendto(sockfd_, payload.data(), payload.length(), 0, (sockaddr *)&ss,
             sslen) != (ssize_t)payload.length()) {
    throw std::runtime_error("sendto() failed");
  }

  if (event_add(evp_, measurement_kit::timeval_init(&tv, timeout)) != 0)
    throw std::runtime_error("event_add() failed");

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

  measurement_kit::debug("on_socket_readable()");

  if (!probe_pending_)
    throw std::runtime_error("No probe is pending");
  probe_pending_ = false;

  r.is_ipv4 = use_ipv4_;

  if (clock_gettime(CLOCK_MONOTONIC, &arr_time) != 0)
    throw std::runtime_error("clock_gettime() failed");
  r.rtt = calculate_rtt(arr_time, start_time_);
  measurement_kit::debug("rtt = %lf", r.rtt);

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
  if ((r.recv_bytes = recvmsg(sockfd_, &msg, MSG_ERRQUEUE)) < 0)
    throw std::runtime_error("recvmsg() failed");
  measurement_kit::debug("recv_bytes = %lu", r.recv_bytes);

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
      measurement_kit::warn("Received unexpected cmsg_type: %d", cmsg->cmsg_type);
      continue;
    }
    if (cmsg->cmsg_type == expected_type_ttl) {
      r.ttl = get_ttl(CMSG_DATA(cmsg));
      measurement_kit::debug("ttl = %d", r.ttl);
      continue;
    }

    // Be robust to refactoring
    assert(cmsg->cmsg_type == expected_type_recverr);

    socket_error = (sock_extended_err *)CMSG_DATA(cmsg);
    if (socket_error->ee_origin != expected_origin) {
      measurement_kit::warn("Received unexpected ee_type: %d", cmsg->cmsg_type);
      continue;
    }
    r.icmp_type = socket_error->ee_type;
    measurement_kit::debug("icmp_type = %d", r.icmp_type);
    r.icmp_code = socket_error->ee_code;
    measurement_kit::debug("icmp_code = %d", r.icmp_code);
    r.interface_ip = get_source_addr(use_ipv4_, socket_error);
    measurement_kit::debug("interface_ip = %s", r.interface_ip.c_str());
  }

  return r;
}

std::string AndroidProber::get_source_addr(bool use_ipv4,
                                           sock_extended_err *se) {
  // Note: I'm not annoyed by this function, if you are feel free to refactor
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
  if (rtt_ms < 0)
    rtt_ms = -1.0; // XXX
  return rtt_ms;
}

void AndroidProber::event_callback(int, short event, void *opaque) {
  AndroidProber *prober = static_cast<AndroidProber *>(opaque);

  measurement_kit::debug("event_callback(_, %d, %p)", event, opaque);

  if ((event & EV_TIMEOUT) != 0) {
    prober->on_timeout();
    prober->timeout_cb_();

  } else if ((event & EV_READ) != 0) {
    ProbeResult result;
    try {
      result = prober->on_socket_readable();
    } catch (std::runtime_error error) {
      prober->error_cb_(error);
      return;
    }
    prober->result_cb_(result);

  } else
    prober->error_cb_(std::runtime_error("Unexpected event error"));
}

void AndroidProber::cleanup() {
  measurement_kit::debug("cleanup(): %p", (void *)this);
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
} // namespace measurement_kit

#endif // __linux__
