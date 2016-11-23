// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/dns.hpp>

namespace mk {
namespace dns {

static struct ClassMapping {
    const char *s;
    QueryClassId id;
} CLASS_MAPPING[] = {
#define XX(nnn) {#nnn, MK_DNS_CLASS_ ## nnn}
    XX(IN),
    XX(CS),
    XX(CH),
    XX(HS),
    {nullptr, MK_DNS_CLASS_INVALID}
#undef XX
};

QueryClass::QueryClass() : id_(MK_DNS_CLASS_INVALID) {}

QueryClass::QueryClass(QueryClassId id) : id_(id) {}

QueryClass::QueryClass(const char *s) : QueryClass(std::string{s}) {}

QueryClass::QueryClass(std::string s) {
    auto m = &CLASS_MAPPING[0];
    while (m->s != nullptr and m->s != s) {
        ++m;
    }
    id_ = m->id; /* With `m->s == nullptr` we have `m->id == INVALID` */
}

bool QueryClass::operator==(QueryClassId id) const {
    return id_ == id;
}

bool QueryClass::operator!=(QueryClassId id) const {
    return id_ != id;
}

QueryClass::operator QueryClassId() const {
    return id_;
}

static struct TypeMapping {
    const char *s;
    QueryTypeId id;
} TYPE_MAPPING[] = {
#define XX(nnn) {#nnn, MK_DNS_TYPE_ ## nnn}
    XX(A),
    XX(NS),
    XX(MD),
    XX(MF),
    XX(CNAME),
    XX(SOA),
    XX(MB),
    XX(MG),
    XX(MR),
    XX(NUL),
    XX(WKS),
    XX(PTR),
    XX(HINFO),
    XX(MINFO),
    XX(MX),
    XX(TXT),
    XX(AAAA),
    XX(REVERSE_A),
    XX(REVERSE_AAAA),
    {nullptr, MK_DNS_TYPE_INVALID}
#undef XX
};

QueryType::QueryType() : id_(MK_DNS_TYPE_INVALID) {}

QueryType::QueryType(QueryTypeId id) : id_(id) {}

QueryType::QueryType(const char *s) : QueryType(std::string{s}) {}

QueryType::QueryType(std::string s) {
    auto m = &TYPE_MAPPING[0];
    while (m->s != nullptr and m->s != s) {
        ++m;
    }
    id_ = m->id; /* With `m->s == nullptr` we have `m->id == INVALID` */
}

bool QueryType::operator==(QueryTypeId id) const {
    return id_ == id;
}

bool QueryType::operator!=(QueryTypeId id) const {
    return id_ != id;
}

QueryType::operator QueryTypeId() const {
    return id_;
}

} // namespace dns
} // namespace mk
