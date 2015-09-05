// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_DNS_RESPONSE_HPP
#define MEASUREMENT_KIT_DNS_RESPONSE_HPP

#include <measurement_kit/common/logger.hpp>

#include <iosfwd>
#include <string>
#include <vector>

namespace measurement_kit {
namespace common {
struct Libs;
}
namespace dns {

using namespace measurement_kit::common;

/// DNS response.
class Response {

  protected:
    int code = 66 /* = DNS_ERR_UNKNOWN */;
    double rtt = 0.0;
    int ttl = 0;
    char type = 0;
    std::vector<std::string> results;

  public:
    Response(Response &) = default;
    Response &operator=(Response &) = default;
    Response(Response &&) = default;
    Response &operator=(Response &&) = default;

    /// Constructs an empty DNS response object.
    Response() {}

    /// Constructs a DNS response object.
    Response(int code, char type, int count, int ttl, double started,
             void *addresses, Logger *lp = Logger::global(),
             Libs *libs = nullptr, int start_from = 0);

    /// Get the results returned by the query.
    std::vector<std::string> get_results() { return results; }

    /// Get whether the response was authoritative.
    std::string get_reply_authoritative() { return "unknown"; /* TODO */ }

    /// Get the integer status code returned by evdns.
    int get_evdns_status() { return code; }

    /// Get the time to live of the response.
    int get_ttl() { return ttl; }

    /// Get the time elapsed since the request was sent until
    /// the response was received.
    double get_rtt() { return rtt; }

    /// Return the evdns type (e.g. DNS_IPv4_A)
    char get_type() { return type; }
};

} // namespace dns
} // namespace measurement_kit
#endif
