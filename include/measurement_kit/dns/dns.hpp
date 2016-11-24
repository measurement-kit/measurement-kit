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
MK_DEFINE_ERR(MK_ERR_DNS(17), InvalidDnsEngine, "")

// getaddrinfo errors
MK_DEFINE_ERR(MK_ERR_DNS(18), TemporaryFailureError, "")
MK_DEFINE_ERR(MK_ERR_DNS(19), InvalidFlagsValueError, "")
MK_DEFINE_ERR(MK_ERR_DNS(20), InvalidHintsValueError, "")
MK_DEFINE_ERR(MK_ERR_DNS(21), NonRecoverableFailureError, "")
MK_DEFINE_ERR(MK_ERR_DNS(22), NotSupportedAIFamilyError, "")
MK_DEFINE_ERR(MK_ERR_DNS(23), MemoryAllocationFailureError, "")
MK_DEFINE_ERR(MK_ERR_DNS(24), HostOrServiceNotProvidedOrNotKnownError, "")
MK_DEFINE_ERR(MK_ERR_DNS(25), ArgumentBufferOverflowError, "")
MK_DEFINE_ERR(MK_ERR_DNS(26), UnknownResolvedProtocolError, "")
MK_DEFINE_ERR(MK_ERR_DNS(27), NotSupportedServnameError, "")
MK_DEFINE_ERR(MK_ERR_DNS(28), NotSupportedAISocktypeError, "")
MK_DEFINE_ERR(MK_ERR_DNS(29), InetNtopFailureError, "")

// c-ares errors
MK_DEFINE_ERR(MK_ERR_DNS(30), SocketCreateError, "")
MK_DEFINE_ERR(MK_ERR_DNS(31), SetsockoptError, "")
MK_DEFINE_ERR(MK_ERR_DNS(32), IntegerOverflowError, "")
MK_DEFINE_ERR(MK_ERR_DNS(33), SendtoError, "")
MK_DEFINE_ERR(MK_ERR_DNS(34), PacketTruncatedError, "")
MK_DEFINE_ERR(MK_ERR_DNS(35), UnexpectedPollFlagsError, "")
MK_DEFINE_ERR(MK_ERR_DNS(36), SocketError, "")
MK_DEFINE_ERR(MK_ERR_DNS(37), UnexpectedShortReadError, "")

// Note: these enums are consistent with the defines in arpa/nameser.h
enum QueryClassId {
    MK_DNS_CLASS_INVALID = 0,
    MK_DNS_CLASS_IN,
    MK_DNS_CLASS_CS,
    MK_DNS_CLASS_CH,
    MK_DNS_CLASS_HS
};

// Note: these enums are consistent with the defines in arpa/nameser.h
enum QueryTypeId {
    MK_DNS_TYPE_INVALID = 0,
    MK_DNS_TYPE_A,
    MK_DNS_TYPE_NS,
    MK_DNS_TYPE_MD,
    MK_DNS_TYPE_MF,
    MK_DNS_TYPE_CNAME,   //  5
    MK_DNS_TYPE_SOA,
    MK_DNS_TYPE_MB,
    MK_DNS_TYPE_MG,
    MK_DNS_TYPE_MR,
    MK_DNS_TYPE_NUL,     // 10
    MK_DNS_TYPE_WKS,
    MK_DNS_TYPE_PTR,
    MK_DNS_TYPE_HINFO,
    MK_DNS_TYPE_MINFO,
    MK_DNS_TYPE_MX,      // 15
    MK_DNS_TYPE_TXT,
    MK_DNS_TYPE_RP,
    MK_DNS_TYPE_AFSDB,
    MK_DNS_TYPE_X25,
    MK_DNS_TYPE_ISDN,    // 20
    MK_DNS_TYPE_RT,
    MK_DNS_TYPE_NSAP,
    MK_DNS_TYPE_NSAP_RP,
    MK_DNS_TYPE_SIG,
    MK_DNS_TYPE_KEY,     // 25
    MK_DNS_TYPE_PX,
    MK_DNS_TYPE_GPOS,
    MK_DNS_TYPE_AAAA,
    MK_DNS_TYPE_LOC,
    MK_DNS_TYPE_NXT,     // 30

    // TODO: map more DNS query types

    MK_DNS_TYPE_REVERSE_A = 65530,   // nonstandard
    MK_DNS_TYPE_REVERSE_AAAA = 65531 // nonstandard
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

} // namespace dns
} // namespace mk
#endif
