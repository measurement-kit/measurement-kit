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

/// Format error
class FormatError : public Error {
  public:
    /// Default constructor
    FormatError() : Error(2000, "dns_lookup_error") {}
};

/// Server failed error
class ServerFailedError : public Error {
  public:
    /// Default constructor
    ServerFailedError() : Error(2001, "dns_lookup_error") {}
};

/// Not exist error
class NotExistError : public Error {
  public:
    /// Default constructor
    NotExistError() : Error(2002, "dns_lookup_error") {}
};

/// Not implemented error
class NotImplementedError : public Error {
  public:
    /// Default constructor
    NotImplementedError() : Error(2003, "dns_lookup_error") {}
};

/// Refused error
class RefusedError : public Error {
  public:
    /// Default constructor
    RefusedError() : Error(2004, "dns_lookup_error") {}
};

/// Truncated error
class TruncatedError : public Error {
  public:
    /// Default constructor
    TruncatedError() : Error(2005, "dns_lookup_error") {}
};

/// Uknown error
class UnknownError : public Error {
  public:
    UnknownError() : Error(2006, "unknown_failure 2006") {}
};

/// Timeout error
class TimeoutError : public Error {
  public:
    TimeoutError() : Error(2007, "generic_timeout_error") {}
};

/// Shutdown error
class ShutdownError : public Error {
  public:
    ShutdownError() : Error(2008, "unknown_failure 2008") {}
};

/// Cancel error
class CancelError : public Error {
  public:
    CancelError() : Error(2009, "unknown_failure 2009") {}
};

/// No data error
class NoDataError : public Error {
  public:
    NoDataError() : Error(2010, "dns_lookup_error") {}
};

/// Invalid IPv4 error
class InvalidIPv4AddressError : public Error {
  public:
    InvalidIPv4AddressError() : Error(2011, "unknown_failure 2011") {}
};

/// Invalid IPv6 error
class InvalidIPv6AddressError : public Error {
  public:
    InvalidIPv6AddressError() : Error(2012, "unknown_failure 2012") {}
};

/// Unsupported class error
class UnsupportedClassError : public Error {
  public:
    UnsupportedClassError() : Error(2013, "unknown_failure 2013") {}
};

/// Invalid name for PTR query error
class InvalidNameForPTRError : public Error {
  public:
    InvalidNameForPTRError() : Error(2014, "unknown_failure 2014") {}
};

/// Resolver error
class ResolverError : public Error {
  public:
    ResolverError() : Error(2015, "unknown_failure 2015") {}
};

/// Unsupported type error
class UnsupportedTypeError : public Error {
  public:
    UnsupportedTypeError() : Error(2016, "unknown_failure 2016") {}
};

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
           Callback<Message> func,
           Settings settings = {},
           Poller *poller = mk::get_global_poller());

} // namespace dns
} // namespace mk
#endif
