// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.

#include "src/libmeasurement_kit/report/entry.hpp"

namespace mk {
namespace report {

/* static */ Entry Entry::array() {
    Entry entry;
    // Manually construct an empty array by pushing
    // a value and then removing it
    entry.push_back(17.0);
    entry.erase(0);
    return entry;
}

/* static */ Entry Entry::object() {
    Entry entry;
    // Manually construct an empty object by pushing
    // a value and then removing it
    entry["foo"] = "bar";
    entry.erase("foo");
    return entry;
}

Entry &Entry::operator=(std::initializer_list<Json> t) {
    Json::operator=(t);
    return *this;
}

void Entry::push_back(Entry value) {
    try {
        Json::push_back(value);
    } catch (const std::domain_error &) {
        throw JsonDomainError();
    }
}

void Entry::push_back(std::initializer_list<Json> j) {
    try {
        Json::push_back(j);
    } catch (const std::domain_error &) {
        throw JsonDomainError();
    }
}

std::string Entry::dump(const int indent) const {
    return Json::dump(indent);
}

Entry Entry::parse(const std::string &s) {
  return static_cast<Entry>(Json::parse(s));
}

bool Entry::operator==(std::nullptr_t right) const noexcept {
    return static_cast<const Json &>(*this) == right;
}

bool Entry::operator!=(std::nullptr_t right) const noexcept {
    return static_cast<const Json &>(*this) != right;
}

} // namespace report
} // namespace mk
