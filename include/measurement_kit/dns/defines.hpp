// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <iosfwd>
#include <stdexcept>
#include <string>

#ifndef MEASUREMENT_KIT_DNS_DEFINES_HPP
#define MEASUREMENT_KIT_DNS_DEFINES_HPP

namespace measurement_kit {
namespace dns {

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
    QueryClass(QueryClassId id) : id_(id) {}

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
    QueryType(QueryTypeId id) : id_(id) {}

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

} // namespace dns
} // namespace measurement_kit
#endif
