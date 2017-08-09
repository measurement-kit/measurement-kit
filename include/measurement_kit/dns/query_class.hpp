// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef MEASUREMENT_KIT_DNS_QUERY_CLASS_HPP
#define MEASUREMENT_KIT_DNS_QUERY_CLASS_HPP

#include <measurement_kit/dns/qctht_.hpp>

namespace mk {
namespace dns {

#define MK_DNS_CLASS_IDS                                                       \
    XX(INVALID) /* Must be first */                                            \
    XX(IN)                                                                     \
    XX(CS)                                                                     \
    XX(CH)                                                                     \
    XX(HS)

#define XX(_name) MK_DNS_CLASS_##_name,
enum QueryClassId { MK_DNS_CLASS_IDS };
#undef XX

QueryClassId query_class_ids_(std::string s);

using QueryClass = qctht_<QueryClassId, query_class_ids_>;

} // namespace dns
} // namespace mk
#endif
