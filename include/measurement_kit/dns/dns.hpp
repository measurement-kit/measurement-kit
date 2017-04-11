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
MK_DEFINE_ERR(MK_ERR_DNS(6), UnknownError, "dns_unknown_error")
MK_DEFINE_ERR(MK_ERR_DNS(7), TimeoutError, "generic_timeout_error")
MK_DEFINE_ERR(MK_ERR_DNS(8), ShutdownError, "dns_shutdown")
MK_DEFINE_ERR(MK_ERR_DNS(9), CancelError, "dns_cancel")
MK_DEFINE_ERR(MK_ERR_DNS(10), NoDataError, "dns_lookup_error")
MK_DEFINE_ERR(MK_ERR_DNS(11), InvalidIPv4AddressError, "dns_invalid_ipv4")
MK_DEFINE_ERR(MK_ERR_DNS(12), InvalidIPv6AddressError, "dns_invalid_ipv6")
MK_DEFINE_ERR(MK_ERR_DNS(13), UnsupportedClassError, "dns_unsupported_class")
MK_DEFINE_ERR(MK_ERR_DNS(14), InvalidNameForPTRError, "dns_invalid_ptr")
MK_DEFINE_ERR(MK_ERR_DNS(15), ResolverError, "dns_resolver_error")
MK_DEFINE_ERR(MK_ERR_DNS(16), UnsupportedTypeError, "dns_unsupported_type")
MK_DEFINE_ERR(MK_ERR_DNS(17), InvalidDnsEngine, "dns_invalid_engine")

// getaddrinfo errors
MK_DEFINE_ERR(MK_ERR_DNS(18), TemporaryFailureError, "dns_temporary_failure")
MK_DEFINE_ERR(MK_ERR_DNS(19), InvalidFlagsValueError, "dns_invalid_flags")
MK_DEFINE_ERR(MK_ERR_DNS(20), InvalidHintsValueError, "dns_invalid_hints")
MK_DEFINE_ERR(MK_ERR_DNS(21), NonRecoverableFailureError, "dns_non_recoverable_failure")
MK_DEFINE_ERR(MK_ERR_DNS(22), NotSupportedAIFamilyError, "dns_unsupported_family")
MK_DEFINE_ERR(MK_ERR_DNS(23), MemoryAllocationFailureError, "dns_memory_error")
MK_DEFINE_ERR(MK_ERR_DNS(24), HostOrServiceNotProvidedOrNotKnownError, "dns_host_or_service_not_provided_or_not_known")
MK_DEFINE_ERR(MK_ERR_DNS(25), ArgumentBufferOverflowError, "dns_overflow_error")
MK_DEFINE_ERR(MK_ERR_DNS(26), UnknownResolvedProtocolError, "dns_unknown_protocol")
MK_DEFINE_ERR(MK_ERR_DNS(27), NotSupportedServnameError, "dns_unsupported_servname")
MK_DEFINE_ERR(MK_ERR_DNS(28), NotSupportedAISocktypeError, "dns_unsupported_socktype")
MK_DEFINE_ERR(MK_ERR_DNS(29), InetNtopFailureError, "dns_inet_ntop_failure")

enum QueryClassId {
    MK_DNS_CLASS_INVALID = 0,
    MK_DNS_CLASS_IN,
    MK_DNS_CLASS_CS,
    MK_DNS_CLASS_CH,
    MK_DNS_CLASS_HS
};

enum QueryTypeId {
    MK_DNS_TYPE_INVALID = 0,
    MK_DNS_TYPE_A,
    MK_DNS_TYPE_NS,
    MK_DNS_TYPE_MD,
    MK_DNS_TYPE_MF,
    MK_DNS_TYPE_CNAME,
    MK_DNS_TYPE_SOA,
    MK_DNS_TYPE_MB,
    MK_DNS_TYPE_MG,
    MK_DNS_TYPE_MR,
    MK_DNS_TYPE_NUL,
    MK_DNS_TYPE_WKS,
    MK_DNS_TYPE_PTR,
    MK_DNS_TYPE_HINFO,
    MK_DNS_TYPE_MINFO,
    MK_DNS_TYPE_MX,
    MK_DNS_TYPE_TXT,
    MK_DNS_TYPE_AAAA,
    MK_DNS_TYPE_REVERSE_A,    // nonstandard
    MK_DNS_TYPE_REVERSE_AAAA  // nonstandard
};

class QueryClass {
  public:
    QueryClass();
    QueryClass(QueryClassId);
    QueryClass(std::string);
    QueryClass(const char *);
    bool operator==(QueryClassId id) const;
    bool operator!=(QueryClassId id) const;
    operator QueryClassId() const;

  private:
    QueryClassId id_;
};

class QueryType {
  public:
    QueryType();
    QueryType(QueryTypeId id);
    QueryType(std::string);
    QueryType(const char *);
    bool operator==(QueryTypeId id) const;
    bool operator!=(QueryTypeId id) const;
    operator QueryTypeId() const;

  private:
    QueryTypeId id_;
};

class Answer {
  public:
    QueryType type;
    QueryClass qclass;
    int code = 0;
    uint32_t ttl = 0;
    std::string name;
    std::string ipv4;             ///< For A records
    std::string ipv6;             ///< For AAAA records
    std::string hostname;         ///< For PTR, SOA and CNAME records
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
    int error_code = 66 /* This is evdns's generic error */;
    std::vector<Answer> answers;
    std::vector<Query> queries;
};

void query(
        QueryClass dns_class,
        QueryType dns_type,
        std::string name,
        Callback<Error, Var<Message>> func,
        Settings settings = {},
        Var<Reactor> reactor = Reactor::global(),
        Var<Logger> logger = Logger::global()
);

struct ResolveHostnameResult {
    bool inet_pton_ipv4 = false;
    bool inet_pton_ipv6 = false;

    Error ipv4_err;
    dns::Message ipv4_reply;
    Error ipv6_err;
    dns::Message ipv6_reply;

    std::vector<std::string> addresses;
};

void resolve_hostname(std::string hostname,
        Callback<ResolveHostnameResult> cb,
        Settings settings = {},
        Var<Reactor> reactor = Reactor::global(),
        Var<Logger> logger = Logger::global());


} // namespace dns
} // namespace mk
#endif
