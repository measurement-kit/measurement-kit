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
MK_DEFINE_ERR(MK_ERR_DNS(36), RecvError, "")
MK_DEFINE_ERR(MK_ERR_DNS(37), UnexpectedShortReadError, "")
MK_DEFINE_ERR(MK_ERR_DNS(38), NoSpaceForQueryError, "")
MK_DEFINE_ERR(MK_ERR_DNS(39), NoSpaceForHeaderError, "")
MK_DEFINE_ERR(MK_ERR_DNS(40), NoSpaceForResourceRecordError, "")
MK_DEFINE_ERR(MK_ERR_DNS(41), InetNtopError, "")
MK_DEFINE_ERR(MK_ERR_DNS(42), InvalidRecordLengthError, "")
MK_DEFINE_ERR(MK_ERR_DNS(43), NoSpaceForResourceRecordHeaderError, "")
MK_DEFINE_ERR(MK_ERR_DNS(44), MalformedEncodedDomainNameError, "")

// Note: these enums are consistent with the defines in arpa/nameser.h
#define MK_DNS_CLASSES                                                         \
    XX(INVALID, 0),                                                            \
    XX(IN, 1),                                                                 \
    XX(CH, 3),                                                                 \
    XX(HS, 4)

enum QueryClassId {
#define XX(codename, value) MK_DNS_CLASS_ ## codename = value
    MK_DNS_CLASSES
#undef XX
};

// Note: these enums are consistent with the defines in arpa/nameser.h
#define MK_DNS_TYPES                                                           \
    XX(INVALID, 0),                                                            \
    XX(A, 1),                                                                  \
    XX(NS, 2),                                                                 \
    XX(CNAME, 5),                                                              \
    XX(SOA, 6),                                                                \
    XX(PTR, 12),                                                               \
    XX(MX, 15),                                                                \
    XX(TXT, 16),                                                               \
    XX(AAAA, 28),                                                              \
    XX(REVERSE_A, 65530), /* nonstandard! */                                   \
    XX(REVERSE_AAAA, 65531) /* nonstandard! */

enum QueryTypeId {
#define XX(codename, value) MK_DNS_TYPE_ ## codename = value
    MK_DNS_TYPES
#undef XX
};

class QueryClass {
  public:
    QueryClass();
    QueryClass(QueryClassId);
    QueryClass(std::string);
    QueryClass(const char *);
    QueryClass(int);
    QueryClass &operator=(const QueryClass &);
    QueryClass &operator=(std::string);
    QueryClass &operator=(const char *);
    QueryClass &operator=(int);
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
    QueryType(int);
    QueryType &operator=(const QueryType &);
    QueryType &operator=(std::string);
    QueryType &operator=(const char *);
    QueryType &operator=(int);
    bool operator==(QueryTypeId id) const;
    bool operator!=(QueryTypeId id) const;
    operator QueryTypeId() const;

  private:
    QueryTypeId id_;
};

class Answer {
  public:
    QueryType type;
    QueryClass aclass;
    int code = 0;
    uint32_t ttl = 0;
    uint16_t dlen = 0;
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
    double rtt = 0.0; /* XXX not honoured by the cares engine */

    /*
     * XXX The following field is a duplicate of rcode.
     */
    int error_code = 66 /* This is evdns's generic error */;

    uint16_t qid;     ///< Query ID
    bool qr;          ///< Query or response
    char opcode;      ///< Operation code
    bool aa;          ///< Authoritative answer
    bool tc;          ///< Truncated content
    bool rd;          ///< Recursion desired
    bool ra;          ///< Recursion available
    bool z;           ///< Zero
    /* TODO: what about the answer authenticated bit? */
    char rcode;       ///< Reply code
    uint16_t qdcount; ///< Questions count
    uint16_t ancount; ///< Answers count
    uint16_t nscount; ///< Name server authority count
    uint16_t arcount; ///< Additional records count

    std::vector<Query> queries;
    std::vector<Answer> answers;
    std::vector<Answer> authorities;
    std::vector<Answer> additionals;
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
