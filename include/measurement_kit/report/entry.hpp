// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_REPORT_ENTRY_HPP
#define MEASUREMENT_KIT_REPORT_ENTRY_HPP

#include <measurement_kit/common.hpp>
#include <measurement_kit/ext.hpp>

namespace mk {
namespace report {

// A report entry. This is implemented using private inheritance from
// nlohmann/json such that we are not exposing externally we are using
// such json library to implement this class.
class Entry : private nlohmann::json {
  public:
    Entry() {}

    // Implementation of dict
    template <typename T> Entry &operator=(T value) {
        nlohmann::json::operator=(value);
        return *this;
    }
    template <typename K> Entry &operator[](const K &key) {
        try {
            return static_cast<Entry &>(nlohmann::json::operator[](key));
        } catch (std::domain_error &) {
            throw DomainError();
        }
    }

    // Implementation of list
    template <typename T> void push_back(T value) {
        try {
            nlohmann::json::push_back(value);
        } catch (std::domain_error &) {
            throw DomainError();
        }
    }

    std::string dump() {
        return nlohmann::json::dump();
    }

  protected:
  private:
    // NO ATTRIBUTES HERE BY DESIGN. DO NOT ADD ATTRIBUTES HERE BECAUSE
    // DOING THAT CREATES THE RISK OF OBJECT SLICING.
};

} // namespace report
} // namespace mk
#endif
