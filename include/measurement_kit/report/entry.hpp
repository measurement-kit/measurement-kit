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
    using nlohmann::json::json;

    static Entry Array() {
        return static_cast<Entry>(nlohmann::json::array());
    }

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
#define XX                                                                     \
        try {                                                                  \
            nlohmann::json::push_back(value);                                  \
        } catch (std::domain_error &) {                                        \
            throw DomainError();                                               \
        }
    template <typename T> void push_back(T value) { XX }
    void push_back(Entry &&value) { XX }
#undef XX

    std::string dump() {
        return nlohmann::json::dump();
    }

    friend bool operator==(Entry &left, std::nullptr_t right) {
        return static_cast<nlohmann::json &>(left) == right;
    }

  protected:
  private:
    // NO ATTRIBUTES HERE BY DESIGN. DO NOT ADD ATTRIBUTES HERE BECAUSE
    // DOING THAT CREATES THE RISK OF OBJECT SLICING.
};

} // namespace report
} // namespace mk
#endif
