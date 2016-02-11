// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_DNS_GETADDRINFO_HPP
#define SRC_DNS_GETADDRINFO_HPP

#include <measurement_kit/common/logger.hpp>
#include <measurement_kit/common/poller.hpp>
#include <measurement_kit/common/maybe.hpp>
#include <measurement_kit/dns/defines.hpp>
#include <functional>
#include <string>

namespace mk {

class Error;

namespace dns {

// Forward declarations
class Response;

/// Issue DNS query using the getaddrinfo() engine.
/// \param qclass Query class.
/// \param qtype Query type.
/// \param qname Name to resolve.
/// \param logger Optional logger.
/// \return Response on success, error on failure.
Maybe<Response> getaddrinfo_query(QueryClass, QueryType, std::string,
                                  Logger * = Logger::global());

} // namespace dns
} // namespace mk
#endif
