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

    static Entry array();

    template <typename T> operator ErrorOr<T>() {
        try {
            return nlohmann::json::operator T();
        } catch (std::domain_error &) {
            return JsonDomainError();
        }
    }
    template <typename T> operator T() {
        try {
            return nlohmann::json::operator T();
        } catch (std::domain_error &) {
            throw JsonDomainError();
        }
    }

    // Implementation of dict
    Entry &operator=(Entry value);
    template <typename K> Entry &operator[](const K &key) {
        // The intent is to only accept string keys but apparently we need
        // to use this template to forward to the real operator
        return operator[](std::string(key));
    }
    Entry &operator[](std::string key);
    constexpr bool is_object() const {
        return nlohmann::json::is_object();
    }

    // Implementation of list
    void push_back(Entry);
    Entry &operator[](int key);
    constexpr bool is_array() const {
        return nlohmann::json::is_array();
    }

    size_t size() const {
        return nlohmann::json::size();
    }
    std::string dump();

    constexpr bool is_null() const {
        return nlohmann::json::is_null();
    }

    bool operator==(Entry right);
    bool operator==(std::nullptr_t right);
    bool operator!=(Entry right);
    bool operator!=(std::nullptr_t right);

  protected:
  private:
    // NO ATTRIBUTES HERE BY DESIGN. DO NOT ADD ATTRIBUTES HERE BECAUSE
    // DOING THAT CREATES THE RISK OF OBJECT SLICING.
};

} // namespace report
} // namespace mk
#endif
