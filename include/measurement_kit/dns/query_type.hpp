// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_DNS_QUERY_TYPE_HPP
#define MEASUREMENT_KIT_DNS_QUERY_TYPE_HPP

#include <string>

namespace mk {
namespace dns {

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

} // namespace dns
} // namespace mk
#endif
