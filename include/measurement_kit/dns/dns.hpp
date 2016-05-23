// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_DNS_DNS_HPP
#define MEASUREMENT_KIT_DNS_DNS_HPP

#include <stdexcept>
#include <string>
#include <vector>

#include <measurement_kit/common.hpp>

struct evdns_base; // Internally we use evdns

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

/// Available query classes id
enum class QueryClassId { IN, CS, CH, HS };

/// Available query types id (REVERSE_A and REVERSE_AAAA are non standard)
enum class QueryTypeId {
    A,
    NS,
    MD,
    MF,
    CNAME,
    SOA,
    MB,
    MG,
    MR,
    NUL,
    WKS,
    PTR,
    HINFO,
    MINFO,
    MX,
    TXT,
    AAAA,
    REVERSE_A,
    REVERSE_AAAA
};

/// Query class
class QueryClass {
  public:
    /// Constructor with id
    QueryClass(QueryClassId id = QueryClassId::IN) : id_(id) {}

    /// Constructor with string
    QueryClass(const char *x) {
        std::string str(x);
        if (str == "IN")
            id_ = QueryClassId::IN;
        else if (str == "CS")
            id_ = QueryClassId::CS;
        else if (str == "CH")
            id_ = QueryClassId::CH;
        else if (str == "HS")
            id_ = QueryClassId::HS;
        else
            throw std::runtime_error("invalid argument");
    }

    /// Equality operator
    bool operator==(QueryClassId id) const { return id_ == id; }

    /// Unequality operator
    bool operator!=(QueryClassId id) const { return id_ != id; }

    /// Cast to query class id
    operator QueryClassId() const { return id_; }

  private:
    QueryClassId id_;
};

/// Query class
class QueryType {
  public:
    /// Constructor with id
    QueryType(QueryTypeId id = QueryTypeId::A) : id_(id) {}

    /// Constructor with string
    QueryType(const char *x) {
        std::string str(x);
        if (str == "A")
            id_ = QueryTypeId::A;
        else if (str == "NS")
            id_ = QueryTypeId::NS;
        else if (str == "MD")
            id_ = QueryTypeId::MD;
        else if (str == "MF")
            id_ = QueryTypeId::MF;
        else if (str == "CNAME")
            id_ = QueryTypeId::CNAME;
        else if (str == "SOA")
            id_ = QueryTypeId::SOA;
        else if (str == "MB")
            id_ = QueryTypeId::MB;
        else if (str == "MG")
            id_ = QueryTypeId::MG;
        else if (str == "MR")
            id_ = QueryTypeId::MR;
        else if (str == "NUL")
            id_ = QueryTypeId::NUL;
        else if (str == "WKS")
            id_ = QueryTypeId::WKS;
        else if (str == "PTR")
            id_ = QueryTypeId::PTR;
        else if (str == "HINFO")
            id_ = QueryTypeId::HINFO;
        else if (str == "MINFO")
            id_ = QueryTypeId::MINFO;
        else if (str == "MX")
            id_ = QueryTypeId::MX;
        else if (str == "TXT")
            id_ = QueryTypeId::TXT;
        else if (str == "AAAA")
            id_ = QueryTypeId::AAAA;
        else if (str == "REVERSE_A")
            id_ = QueryTypeId::REVERSE_A;
        else if (str == "REVERSE_AAAA")
            id_ = QueryTypeId::REVERSE_AAAA;
        else
            throw std::runtime_error("invalid argument");
    }

    /// Equality operator
    bool operator==(QueryTypeId id) const { return id_ == id; }

    /// Unequality operator
    bool operator!=(QueryTypeId id) const { return id_ != id; }

    /// Cast to query class id
    operator QueryTypeId() const { return id_; }

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

        std::string ipv4; ///< For A records
        std::string ipv6; ///< For AAAA records

        std::string hostname; ///< For PTR, SOA and CNAME records

        std::string responsible_name; ///< For SOA records
        uint32_t serial_number; ///< For SOA records
        uint32_t refresh_interval; ///< For SOA records
        uint32_t retry_interval; ///< For SOA records
        uint32_t minimum_ttl; ///< For SOA records
        uint32_t expiration_limit; ///< For SOA records

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
        Message() {};
        Message(std::nullptr_t) {};

        double rtt = 0.0;

        int error_code = 66;

        std::vector<Answer> answers;
        std::vector<Query> queries;
};

/// Perform a single DNS query
void query(QueryClass dns_class, QueryType dns_type, std::string name,
           Callback<Error, Message> func,
           Settings settings = {},
           Var<Reactor> reactor = Reactor::global());

} // namespace dns
} // namespace mk
#endif
