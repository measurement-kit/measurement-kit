// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/report.hpp>

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

Entry &Entry::operator=(Entry value) {
    nlohmann::json::operator=(value);
    return *this;
}

Entry &Entry::operator[](std::string key) {
    // Note: out_of_range could not happen because operator[] returns
    // a null json object if the accessed key is missing
    try {
        return static_cast<Entry &>(nlohmann::json::operator[](key));
    } catch (std::domain_error &) {
        throw JsonDomainError();
    }
}

void Entry::push_back(Entry value) {
    try {
        nlohmann::json::push_back(value);
    } catch (std::domain_error &) {
        throw JsonDomainError();
    }
}

std::string Entry::dump() {
    return nlohmann::json::dump();
}

bool Entry::operator==(std::nullptr_t right) {
    return static_cast<nlohmann::json &>(*this) == right;
}

bool Entry::operator!=(std::nullptr_t right) {
    return static_cast<nlohmann::json &>(*this) != right;
}

} // namespace report
} // namespace mk
