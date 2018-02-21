// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_DNS_QUERY_TYPE_HPP
#define SRC_LIBMEASUREMENT_KIT_DNS_QUERY_TYPE_HPP

#include "src/libmeasurement_kit/dns/qctht_.hpp"

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

using QueryType = qctht_<QueryTypeId, query_type_ids_>;

} // namespace dns
} // namespace mk
#endif
