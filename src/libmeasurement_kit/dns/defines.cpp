// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/dns.hpp>

namespace mk {
namespace dns {

/*
 * Class
 */

struct ClassMapping {
    const char      *as_string;
    QueryClassId     as_enum;
    int              as_int;
};

static const std::vector<ClassMapping> get_classes() {
    static std::vector<ClassMapping> singleton = {
#define XX(codename, value) {#codename, MK_DNS_CLASS_ ## codename, value}
        MK_DNS_CLASSES
#undef XX
    };
    return singleton;
}

QueryClass::QueryClass() : id_(MK_DNS_CLASS_INVALID) {}

QueryClass::QueryClass(QueryClassId id) : id_(id) {}

QueryClass::QueryClass(const char *s) : QueryClass(std::string{s}) {}

#define XX(_type_, _compare_field_)                                            \
    QueryClass::QueryClass(_type_ x) {                                         \
        for (auto &m : get_classes()) {                                        \
            if (m._compare_field_ == x) {                                      \
                id_ = m.as_enum;                                               \
                return;                                                        \
            }                                                                  \
        }                                                                      \
        id_ = MK_DNS_CLASS_INVALID;                                            \
    }

XX(std::string, as_string)
XX(int, as_int)

#undef XX

QueryClass &QueryClass::operator=(const QueryClass &other) {
    id_ = other.id_;
    return *this;
}

QueryClass &QueryClass::operator=(std::string s) {
    *this = QueryClass{s};
    return *this;
}

QueryClass &QueryClass::operator=(const char *s) {
    *this = QueryClass{s};
    return *this;
}

QueryClass &QueryClass::operator=(int d) {
    *this = QueryClass{d};
    return *this;
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

/*
 * Type
 */

struct TypeMapping {
    const char      *as_string;
    QueryTypeId      as_enum;
    int              as_int;
};

static const std::vector<TypeMapping> get_types() {
    static std::vector<TypeMapping> singleton = {
#define XX(codename, value) {#codename, MK_DNS_TYPE_ ## codename, value}
        MK_DNS_TYPES
#undef XX
    };
    return singleton;
}

QueryType::QueryType() : id_(MK_DNS_TYPE_INVALID) {}

QueryType::QueryType(QueryTypeId id) : id_(id) {}

QueryType::QueryType(const char *s) : QueryType(std::string{s}) {}

#define XX(_type_, _compare_field_)                                            \
    QueryType::QueryType(_type_ x) {                                           \
        for (auto &m : get_types()) {                                          \
            if (m._compare_field_ == x) {                                      \
                id_ = m.as_enum;                                               \
                return;                                                        \
            }                                                                  \
        }                                                                      \
        id_ = MK_DNS_TYPE_INVALID;                                             \
    }

XX(std::string, as_string)
XX(int, as_int)

#undef XX

QueryType &QueryType::operator=(const QueryType &other) {
    id_ = other.id_;
    return *this;
}

QueryType &QueryType::operator=(std::string s) {
    *this = QueryType{s};
    return *this;
}

QueryType &QueryType::operator=(const char *s) {
    *this = QueryType{s};
    return *this;
}

QueryType &QueryType::operator=(int d) {
    *this = QueryType{d};
    return *this;
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
