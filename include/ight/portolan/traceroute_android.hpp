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
#ifndef LIBIGHT_PORTOLAN_TRACEROUTE_ANDROID_HPP
#define LIBIGHT_PORTOLAN_TRACEROUTE_ANDROID_HPP

//
// This file contains the primitives needed to run a traceroute
// on Android platforms (and other Linux-based platforms).
//

// We use this on Android and compile this on all Linuxes
#ifdef __linux__

#include <ight/common/constraints.hpp>
#include <ight/common/pointer.hpp>

#include <event2/event.h>

#include <time.h>

#include <functional>
#include <string>

struct sock_extended_err; // Forward declaration

namespace ight {
namespace portolan {
namespace traceroute_android {

using namespace ight::common::constraints;
using namespace ight::common::pointer;

/// Meaning of a probe result
enum class Meaning {
    OTHER = 0,            ///< Another meaning
    NO_ROUTE_TO_HOST = 1, ///< No route to host
    ADDRESS_UNREACH = 2,  ///< E.g., link down
    PROTO_NOT_IMPL = 3,   ///< UDP not implemented
    DEST_REACHED = 4,     ///< Port is closed = dest. reached
    TTL_EXCEEDED = 5,     ///< TTL is too small
    ADMIN_FILTER = 6,     ///< E.g., firewall rule
};

/// Result of a traceroute probe
struct ProbeResult {
    std::string interface_ip;      ///< Host that replied
    int ttl = 0;                   ///< Response TTL
    double rtt = 0.0;              ///< Round trip time
    bool is_ipv4 = true;           ///< Are we using IPv4?
    unsigned char icmp_type = 255; ///< Raw ICMP/ICMPv6 type
    unsigned char icmp_code = 255; ///< Raw ICMP/ICMPv6 code
    ssize_t recv_bytes = 0;        ///< Bytes recv'd

    /// Maps ICMP/ICMPv6 type and code to a meaning
    Meaning get_meaning();
};

/// Sends UDP pings with specified TTL
class Prober : public NonCopyable, public NonMovable {

  private:
    int sockfd = -1;              ///< socket descr
    bool probe_pending = false;   ///< probe is pending
    timespec start_time{};        ///< start time
    bool use_ipv4 = true;         ///< using IPv4?
    event_base *evbase = nullptr; ///< event base
    event *evp = nullptr;         ///< event pointer

    std::function<void(ProbeResult)> result_cb;       ///< on result callback
    std::function<void()> timeout_cb;                 ///< on timeout callback
    std::function<void(std::runtime_error)> error_cb; ///< on error callback

    /// Returns the source address of the error message.
    /// \param use_ipv4 whether we are using IPv4
    /// \param err socket error structure
    /// \return source address of the error message
    static std::string get_source_addr(bool use_ipv4, sock_extended_err *err);

    /// Returns the Round Trip Time value in milliseconds
    /// \param end ICMP reply arrival time
    /// \param start UDP probe send time
    /// \return RTT value in milliseconds
    static double calculate_rtt(struct timespec end, struct timespec start);

    /// Returns the Time to Live of the error message
    /// \param data CMSG_DATA(cmsg)
    /// \return ttl of the error message
    static int get_ttl(void *data) { return *((int *)data); }

    /// Callback invoked when the socket is readable
    /// \param so Socket descriptor
    /// \param event Event that occurred
    /// \param ptr Opaque pointer to this class
    static void event_callback(int so, short event, void *ptr);

    /// Private constructor to enforce using `open()`
    /// \param use_ipv4 Whether to use IPv4
    /// \param port The port to bind
    /// \param evbase Event base to use (optional)
    /// \throws Exception on error
    Prober(bool use_ipv4, int port, event_base *evbase = nullptr);

  public:
    /// Destroys the prober
    ~Prober() { close(); }

    /// Idempotent cleanup function
    /// \warning Not calling this function when you are done with this
    /// object may cause memory leaks
    void close();

    /// Get the underlying UDP socket
    /// \throws Exception on error
    /// \remark This function name finishes with underscore to signal
    /// that it should not be invoked directly unless you want to
    /// implement your own poll() based loop
    int get_socket_() {
        if (sockfd < 0)
            throw std::runtime_error("Programmer error");
        return sockfd;
    }

    /// Send a traceroute probe
    /// \param addr Destination address
    /// \param port Destination port
    /// \param ttl Time to live
    /// \param payload packet payload
    /// \throws Exception on error
    void send_probe(std::string addr, int port, int ttl, std::string payload);

    /// Call this when you don't receive a response within timeout
    /// \remark This function name finishes with underscore to signal
    /// that it should not be invoked directly unless you want to
    /// implement your own poll() based loop
    void on_timeout_() { probe_pending = false; }

    /// Call this as soon as the socket is readable to get
    /// the result ICMP error received by the socket and to
    /// calculate *precisely* the RTT.
    /// \remark This function name finishes with underscore to signal
    /// that it should not be invoked directly unless you want to
    /// implement your own poll() based loop
    /// \throws Exception on error
    ProbeResult on_socket_readable_();

    /// Set callback called when result is available
    void on_result(std::function<void(ProbeResult)> cb) { result_cb = cb; }

    /// Set callback called on timeout
    void on_timeout(std::function<void()> cb) { timeout_cb = cb; }

    /// Set callback called when there is an error
    void on_error(std::function<void(std::runtime_error)> cb) { error_cb = cb; }

    /// Constructs movable/copiable prober
    /// \param use_ipv4 Whether to use IPv4
    /// \param port The port to bind
    /// \param evbase Event base to use
    /// \throws Exception on error
    static SharedPointer<Prober> open(bool use_ipv4, int port,
                                      event_base *evbase = nullptr) {
        return SharedPointer<Prober>{new Prober(use_ipv4, port, evbase)};
    }
};

}}}
#endif
#endif
