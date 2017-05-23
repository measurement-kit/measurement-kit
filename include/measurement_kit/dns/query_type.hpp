// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_DNS_QUERY_TYPE_HPP
#define MEASUREMENT_KIT_DNS_QUERY_TYPE_HPP

#include <measurement_kit/dns/qctht_.hpp>

namespace mk {
namespace dns {

#define MK_DNS_TYPE_IDS                                                        \
    XX(INVALID) /* Must be first */                                            \
    XX(A)                                                                      \
    XX(NS)                                                                     \
    XX(MD)                                                                     \
    XX(MF)                                                                     \
    XX(CNAME)                                                                  \
    XX(SOA)                                                                    \
    XX(MB)                                                                     \
    XX(MG)                                                                     \
    XX(MR)                                                                     \
    XX(NUL)                                                                    \
    XX(WKS)                                                                    \
    XX(PTR)                                                                    \
    XX(HINFO)                                                                  \
    XX(MINFO)                                                                  \
    XX(MX)                                                                     \
    XX(TXT)                                                                    \
    XX(AAAA)                                                                   \
    XX(REVERSE_A /* nonstandard */)                                            \
    XX(REVERSE_AAAA /* nonstandard */)

#define XX(_name) MK_DNS_TYPE_##_name,
enum QueryTypeId { MK_DNS_TYPE_IDS };
#undef XX

QueryTypeId query_type_ids_(std::string s);

class QueryType : public qctht_<QueryTypeId, query_type_ids_> {
  public:
    using qctht_<QueryTypeId, query_type_ids_>::qctht_;
};

} // namespace dns
} // namespace mk
#endif
