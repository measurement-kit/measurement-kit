/*-
 * This file is part of Libight <https://libight.github.io/>.
 *
 * Libight is free software. See AUTHORS and LICENSE for more
 * information on the copying conditions.
 *
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
#ifndef IGHT_TRACEROUTE_ANDROID_HPP
#define IGHT_TRACEROUTE_ANDROID_HPP

/// Android implementation of traceroute interface

// This is meant to run on Android but can run on all Linux systems
#ifdef __linux__

#include <ight/common/constraints.hpp>
#include <ight/traceroute/interface.hpp>

#include <event2/event.h>

#include <time.h>

struct sock_extended_err; // Forward declaration

namespace ight {
namespace traceroute {

/// Traceroute prober for Android
class AndroidProber : public ight::common::constraints::NonCopyable,
                      public ight::common::constraints::NonMovable,
                      public ProberInterface {

public:
  /// Constructor
  /// \param use_ipv4 Whether to use IPv4
  /// \param port The port to bind
  /// \param evbase Event base to use (optional)
  /// \throws Exception on error
  AndroidProber(bool use_ipv4, int port,
                event_base *evbase = ight_get_global_event_base());

  /// Destructor
  ~AndroidProber() { cleanup(); }

  virtual void send_probe(std::string addr, int port, int ttl,
                          std::string payload, double timeout) final;

  virtual void on_result(std::function<void(ProbeResult)> cb) final {
    result_cb_ = cb;
  }

  virtual void on_timeout(std::function<void()> cb) final { timeout_cb_ = cb; }

  virtual void on_error(std::function<void(std::runtime_error)> cb) final {
    error_cb_ = cb;
  }

private:
  int sockfd_ = -1;              ///< socket descr
  bool probe_pending_ = false;   ///< probe is pending
  timespec start_time_{0, 0};    ///< start time
  bool use_ipv4_ = true;         ///< using IPv4?
  event_base *evbase_ = nullptr; ///< event base
  event *evp_ = nullptr;         ///< event pointer

  std::function<void(ProbeResult)> result_cb_;       ///< on result callback
  std::function<void()> timeout_cb_;                 ///< on timeout callback
  std::function<void(std::runtime_error)> error_cb_; ///< on error callback

  /// Call this when you don't receive a response within timeout
  void on_timeout() { probe_pending_ = false; }

  /// Call this as soon as the socket is readable to get
  /// the result ICMP error received by the socket and to
  /// calculate *precisely* the RTT.
  ProbeResult on_socket_readable();

  /// Returns the source address of the error message.
  /// \param use_ipv4 whether we are using IPv4
  /// \param err socket error structure
  /// \return source address of the error message
  static std::string get_source_addr(bool use_ipv4, sock_extended_err *err);

  /// Returns the Round Trip Time value in milliseconds
  /// \param end ICMP reply arrival time
  /// \param start UDP probe send time
  /// \return RTT value in milliseconds
  static double calculate_rtt(timespec end, timespec start);

  /// Returns the Time to Live of the error message
  /// \param data CMSG_DATA(cmsg)
  /// \return ttl of the error message
  static int get_ttl(void *data) { return *((int *)data); }

  /// Callback invoked when the socket is readable
  /// \param so Socket descriptor
  /// \param event Event that occurred
  /// \param ptr Opaque pointer to this class
  static void event_callback(int so, short event, void *ptr);

  /// Idempotent cleanup function
  void cleanup();
};

} // namespace traceroute
} // namespace ight
#endif
#endif
