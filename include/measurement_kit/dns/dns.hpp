// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_DNS_DNS_HPP
#define MEASUREMENT_KIT_DNS_DNS_HPP

#include <measurement_kit/common.hpp>

namespace mk {
namespace dns {

MK_DEFINE_ERR(MK_ERR_DNS(0), FormatError, "dns_lookup_error")
MK_DEFINE_ERR(MK_ERR_DNS(1), ServerFailedError, "dns_lookup_error")
MK_DEFINE_ERR(MK_ERR_DNS(2), NotExistError, "dns_lookup_error")
MK_DEFINE_ERR(MK_ERR_DNS(3), NotImplementedError, "dns_lookup_error")
MK_DEFINE_ERR(MK_ERR_DNS(4), RefusedError, "dns_lookup_error")
MK_DEFINE_ERR(MK_ERR_DNS(5), TruncatedError, "dns_lookup_error")
MK_DEFINE_ERR(MK_ERR_DNS(6), UnknownError, "")
MK_DEFINE_ERR(MK_ERR_DNS(7), TimeoutError, "generic_timeout_error")
MK_DEFINE_ERR(MK_ERR_DNS(8), ShutdownError, "")
MK_DEFINE_ERR(MK_ERR_DNS(9), CancelError, "")
MK_DEFINE_ERR(MK_ERR_DNS(10), NoDataError, "dns_lookup_error")
MK_DEFINE_ERR(MK_ERR_DNS(11), InvalidIPv4AddressError, "")
MK_DEFINE_ERR(MK_ERR_DNS(12), InvalidIPv6AddressError, "")
MK_DEFINE_ERR(MK_ERR_DNS(13), UnsupportedClassError, "")
MK_DEFINE_ERR(MK_ERR_DNS(14), InvalidNameForPTRError, "")
MK_DEFINE_ERR(MK_ERR_DNS(15), ResolverError, "")
MK_DEFINE_ERR(MK_ERR_DNS(16), UnsupportedTypeError, "")

Error dns_error(int code); ///< Map evdns code to proper error

using QueryType = std::string;
using QueryClass = std::string;

class Answer {
  public:
    QueryType type;
    QueryClass qclass;

    int code = 0;

    uint32_t ttl = 0;

    std::string name;

    std::string ipv4; ///< For A records
    std::string ipv6; ///< For AAAA records

    std::string hostname; ///< For PTR, SOA and CNAME records

    std::string responsible_name; ///< For SOA records
    uint32_t serial_number;       ///< For SOA records
    uint32_t refresh_interval;    ///< For SOA records
    uint32_t retry_interval;      ///< For SOA records
    uint32_t minimum_ttl;         ///< For SOA records
    uint32_t expiration_limit;    ///< For SOA records
};

class Query {
  public:
    QueryType type;
    QueryClass qclass;

    uint32_t ttl = 0;

    std::string name;
};

class Message {
  public:
    Message(){};
    Message(std::nullptr_t){};

    double rtt = 0.0;

    int error_code = 66;

    std::vector<Answer> answers;
    std::vector<Query> queries;
};

/// Perform a single DNS query
void query(QueryClass dns_class, QueryType dns_type, std::string name,
           Callback<Error, Message> func, Settings settings = {},
           Var<Reactor> reactor = Reactor::global());

} // namespace dns
} // namespace mk
#endif
