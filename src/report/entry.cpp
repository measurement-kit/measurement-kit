// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.

#include <measurement_kit/report.hpp>

namespace mk {
namespace report {

/* static */ Entry Entry::Array() {
    Entry entry;
    entry.push_back(17.0);
    entry.erase(0);
    return entry;
}

Entry &Entry::operator=(Entry value) {
    nlohmann::json::operator=(value);
    return *this;
}

void Entry::push_back(Entry value) {
    try {
        nlohmann::json::push_back(value);
    } catch (std::domain_error &) {
        throw DomainError();
    }
}

std::string Entry::dump() {
    return nlohmann::json::dump();
}

} // namespace report
} // namespace mk
