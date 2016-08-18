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
#ifndef MEASUREMENT_KIT_TRACEROUTE_ANDROID_HPP
#define MEASUREMENT_KIT_TRACEROUTE_ANDROID_HPP

/// Android implementation of traceroute interface

// This is meant to run on Android but can run on all Linux systems
#ifdef __linux__

#include <measurement_kit/common.hpp>
#include <measurement_kit/traceroute/interface.hpp>

#include <functional>
#include <string>

#include <time.h>

struct event;
struct event_base;
struct sock_extended_err;
struct sockaddr_in6;
struct sockaddr_in;
struct sockaddr_storage;

namespace mk {

class Error;

namespace traceroute {

/// Traceroute prober for Android
class AndroidProber : public NonCopyable,
                      public NonMovable,
                      public ProberInterface {

  public:
    /// Constructor
    /// \param use_ipv4 Whether to use IPv4
    /// \param port The port to bind
    /// \param evbase Event base to use (optional)
    AndroidProber(
        bool use_ipv4, int port,
        event_base *evbase = Reactor::global()->get_event_base(),
        Var<Logger> logger = Logger::global());

    /// Destructor
    ~AndroidProber() { cleanup(); }

    void send_probe(std::string addr, int port, int ttl, std::string payload,
                    double timeout) override;

    void on_result(std::function<void(ProbeResult)> cb) override {
        result_cb_ = cb;
    }

    void on_timeout(std::function<void()> cb) override { timeout_cb_ = cb; }

    void on_error(std::function<void(Error)> cb) override {
        error_cb_ = cb;
    }

  private:
    int sockfd_ = -1;              ///< socket descr
    bool probe_pending_ = false;   ///< probe is pending
    timespec start_time_{0, 0};    ///< start time
    bool use_ipv4_ = true;         ///< using IPv4?
    event_base *evbase_ = nullptr; ///< event base
    event *evp_ = nullptr;         ///< event pointer
    int port_ = 0;                 ///< socket port
    Var<Logger> logger = Logger::global();///< logger

    Delegate<ProbeResult> result_cb_;  ///< on result callback
    Delegate<> timeout_cb_;            ///< on timeout callback
    Delegate<Error> error_cb_;         ///< on error callback

    /// Call this when you don't receive a response within timeout
    void on_timeout() { probe_pending_ = false; }

    /// Initialize socket
    void init();

    /// Call this as soon as the socket is readable to get
    /// the result ICMP error received by the socket and to
    /// calculate *precisely* the RTT.
    ProbeResult on_socket_readable();

    /// Returns the source address of the error message.
    /// \param s IPv4 socket address
    /// \return source address of the error message
    static std::string get_source_addr(const sockaddr_in *s);

    /// Returns the source address of the error message.
    /// \param s IPv6 socket address
    /// \return source address of the error message
    static std::string get_source_addr(const sockaddr_in6 *s);

    /// Returns the source address of the error message.
    /// \param use_ipv4 whether we are using IPv4
    /// \param ss Pointer to sockaddr_storage struct
    /// \return source address of the error message
    static std::string get_source_addr(bool use_ipv4,
                                       const sockaddr_storage *ss);

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
} // namespace mk
#endif
#endif
