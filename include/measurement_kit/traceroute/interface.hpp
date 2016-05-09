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
#ifndef MEASUREMENT_KIT_TRACEROUTE_INTERFACE_HPP
#define MEASUREMENT_KIT_TRACEROUTE_INTERFACE_HPP

/// Interface of traceroute module

// Disable for non Linux until we figure out how to build on iOS
#ifdef __linux__

#include <measurement_kit/common/reactor.hpp>

#include <functional>
#include <memory>
#include <stdexcept>
#include <string>

#include <sys/types.h>

// Forward declarations
struct event_base;

namespace mk {

class Error;

namespace traceroute {

/// Meaning of a probe result
enum class ProbeResultMeaning {
    OTHER = 0,            ///< Another meaning
    NO_ROUTE_TO_HOST = 1, ///< No route to host
    ADDRESS_UNREACH = 2,  ///< E.g., link down
    PROTO_NOT_IMPL = 3,   ///< UDP not implemented
    PORT_IS_CLOSED = 4,   ///< Port is closed
    TTL_EXCEEDED = 5,     ///< TTL is too small
    ADMIN_FILTER = 6,     ///< E.g., firewall rule
    GOT_REPLY_PACKET = 7, ///< We got a real reply packet
};

/// Result of a traceroute probe
class ProbeResult {
  public:
    std::string interface_ip;      ///< Host that replied
    int ttl = 0;                   ///< Response TTL
    double rtt = 0.0;              ///< Round trip time
    bool is_ipv4 = true;           ///< Are we using IPv4?
    unsigned char icmp_type = 255; ///< Raw ICMP/ICMPv6 type
    unsigned char icmp_code = 255; ///< Raw ICMP/ICMPv6 code
    ssize_t recv_bytes = 0;        ///< Bytes recv'd
    bool valid_reply = false;      ///< Whether reply is valid
    std::string reply;             ///< Reply packet data

    /// Maps ICMP/ICMPv6 type and code to a meaning
    ProbeResultMeaning get_meaning();
};

/// Interface of a prober
class ProberInterface {

  public:
    /// Default constructor
    ProberInterface() {}

    /// Send a traceroute probe
    /// In case of error this function emits error-event
    /// \param addr Destination address
    /// \param port Destination port
    /// \param ttl Time to live
    /// \param payload packet payload
    /// \param timeout Timeout for this probe
    virtual void send_probe(std::string addr, int port, int ttl,
                            std::string payload, double timeout) = 0;

    /// Set callback called when result is available
    virtual void on_result(std::function<void(ProbeResult)> cb) = 0;

    /// Set callback called on timeout
    virtual void on_timeout(std::function<void()> cb) = 0;

    /// Set callback called when there is an error
    virtual void on_error(std::function<void(Error)> cb) = 0;

    /// Default copy constructor
    ProberInterface(ProberInterface &) = default;

    /// Default assignment operator
    ProberInterface &operator=(ProberInterface &) = default;

    /// Default move constructor
    ProberInterface(ProberInterface &&) = default;

    /// Default move assignment operator
    ProberInterface &operator=(ProberInterface &&) = default;

    /// Default destructor
    virtual ~ProberInterface();
};

/// Proxy for a prober implementation
template <class Impl> class Prober : public ProberInterface {

  public:
    /// Constructor
    /// \param use_ipv4 Whether to use IPv4
    /// \param port The port to bind
    /// \param evbase Event base to use (optional)
    /// \throws Exception on error
    Prober(bool use_ipv4, int port,
           event_base *evbase = Reactor::global()->get_event_base()) {
        impl_.reset(new Impl(use_ipv4, port, evbase));
    }

    void send_probe(std::string addr, int port, int ttl, std::string payload,
                    double timeout) override {
        impl_->send_probe(addr, port, ttl, payload, timeout);
    }

    void on_result(std::function<void(ProbeResult)> cb) override {
        impl_->on_result(cb);
    }

    void on_timeout(std::function<void()> cb) override {
        impl_->on_timeout(cb);
    }

    void on_error(std::function<void(Error)> cb) override {
        impl_->on_error(cb);
    }

  private:
    std::unique_ptr<Impl> impl_;
};

} // namespace traceroute
} // namespace mk
#endif
#endif
