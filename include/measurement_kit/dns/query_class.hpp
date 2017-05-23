// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_DNS_QUERY_CLASS_HPP
#define MEASUREMENT_KIT_DNS_QUERY_CLASS_HPP

#include <string>

namespace mk {
namespace dns {

enum QueryClassId {
    MK_DNS_CLASS_INVALID = 0,
    MK_DNS_CLASS_IN,
    MK_DNS_CLASS_CS,
    MK_DNS_CLASS_CH,
    MK_DNS_CLASS_HS
};

class QueryClass {
  public:
    QueryClass();
    QueryClass(QueryClassId);
    QueryClass(std::string);
    QueryClass(const char *);
    bool operator==(QueryClassId id) const;
    bool operator!=(QueryClassId id) const;
    operator QueryClassId() const;

  private:
    QueryClassId id_;
};

} // namespace dns
} // namespace mk
#endif
