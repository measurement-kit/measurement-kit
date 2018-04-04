// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_DNS_QUERY_CLASS_HPP
#define SRC_LIBMEASUREMENT_KIT_DNS_QUERY_CLASS_HPP

#include "src/libmeasurement_kit/dns/qctht_.hpp"

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
