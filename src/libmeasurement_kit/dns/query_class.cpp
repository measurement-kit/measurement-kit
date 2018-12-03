// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/dns/query_class.hpp"

namespace mk {
namespace dns {

#define XX(_name)                                                              \
    if (s == #_name) {                                                         \
        return MK_DNS_CLASS_##_name;                                           \
    }
QueryClassId query_class_ids_(std::string s) {
    MK_DNS_CLASS_IDS
    return MK_DNS_CLASS_INVALID;
}
#undef XX

} // namespace dns
} // namespace mk
