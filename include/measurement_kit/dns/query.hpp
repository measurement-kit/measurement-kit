// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#ifndef MEASUREMENT_KIT_DNS_QUERY_HPP
#define MEASUREMENT_KIT_DNS_QUERY_HPP

#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/pointer.hpp>

#include <functional>
#include <iosfwd>

struct evdns_base; // Internally we use evdns

namespace measurement_kit {
namespace common {
struct Libs;
}
namespace dns {

class Response;

using namespace measurement_kit::common;

/// Async DNS request.
class Query {

  protected:
    SharedPointer<bool> cancelled;

  public:
     /// Default constructor.
    Query() {}

    /// Start an async DNS request.
    Query(std::string query, std::string address,
          std::function<void(Response)> func,
          Logger *lp = Logger::global(), evdns_base *dnsb = nullptr,
          Libs *libs = nullptr);

    Query(Query &) = default;
    Query &operator=(Query &) = default;
    Query(Query &&) = default;
    Query &operator=(Query &&) = default;

    /// Cancel the pending Query.
    void cancel();

    /// Destructor.
    ~Query() { cancel(); }
};

} // namespace dns
} // namespace measurement_kit
#endif
